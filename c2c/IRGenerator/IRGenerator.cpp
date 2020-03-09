/* Copyright 2013-2020 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "IRGenerator/IRGenerator.h"
#include "IRGenerator/CodeGenModule.h"
#include "Utils/color.h"
#include "Utils/Utils.h"
#include "Utils/Log.h"

//#include <stdio.h>
#include <pthread.h>
#include <vector>

const unsigned MAX_THREADS = 32;

using namespace C2;

class BuildQueue {
public:
    BuildQueue(bool verbose_, bool print_, bool keep_, OptimizationLevel opt_)
        : index(0)
        , failed(false)
        , verbose(verbose_)
        , printTiming(print_)
        , keep_intermediates(keep_)
        , opt(opt_)
    {
        pthread_mutex_init(&lock, 0);
    }
    ~BuildQueue() {
        for (unsigned i=0; i<queue.size(); ++i) delete queue[i];
    }

    void add(CodeGenModule* cgm) {
        // NOTE: all adding is done single-threaded
        queue.push_back(cgm);
    }
    bool hasFailed() const { return failed; }
    unsigned size() const { return queue.size(); }
    CodeGenModule* get() {
        CodeGenModule* cgm = 0;
        pthread_mutex_lock(&lock);
        if (index < queue.size()) {
            cgm = queue[index];
            index++;
        }
        pthread_mutex_unlock(&lock);
        return cgm;
    }
    void fail() {
        pthread_mutex_lock(&lock);
        failed = true;
        pthread_mutex_unlock(&lock);
    }
    bool getVerbose() const { return verbose; }
    bool getPrintTiming() const { return printTiming; }
    bool getKeepIntermediates() const { return keep_intermediates; }
    OptimizationLevel getOptimizationLevel() const { return opt; }
private:
    pthread_mutex_t lock;
    unsigned index;
    bool failed;
    typedef std::vector<CodeGenModule*> Queue;
    Queue queue;

    const bool verbose;
    const bool printTiming;
    const bool keep_intermediates;
    const OptimizationLevel opt;
};


class IrWorker {
public:
    IrWorker(unsigned idx, BuildQueue& queue_)
        : index(idx)
        , queue(queue_)
    {
        pthread_create(&thread, 0, thread_main, this);
    }
    ~IrWorker() {}

    void join() {
        pthread_join(thread, 0);
    }
    static bool build_module(CodeGenModule& cgm, bool verbose, bool printTiming, bool keep, OptimizationLevel opt) {
        if (verbose) Log::log(COL_VERBOSE, "generating IR (%s)", cgm.getName().c_str());
        uint64_t t1a = Utils::getCurrentTime();
        cgm.generate();
        //if (printIR) cgm.dump();
        if (!cgm.verify()) return false;
        cgm.write();
        uint64_t t1b = Utils::getCurrentTime();
        if (printTiming) Log::log(COL_TIME, "IR generation (%s) took %" PRIu64" usec", cgm.getName().c_str(), t1b - t1a);

        if (verbose) Log::log(COL_VERBOSE, "optimizing IR (%s)", cgm.getName().c_str());
        uint64_t t2a = Utils::getCurrentTime();
        if (!cgm.optimize(opt)) return false;
        uint64_t t2b = Utils::getCurrentTime();
        if (printTiming) Log::log(COL_TIME, "IR optimization (%s) took %" PRIu64" usec", cgm.getName().c_str(), t2b - t2a);

        if (verbose) Log::log(COL_VERBOSE, "compiling IR (%s)", cgm.getName().c_str());
        uint64_t t3a = Utils::getCurrentTime();
        if (!cgm.compile()) return false;
        uint64_t t3b = Utils::getCurrentTime();
        if (printTiming) Log::log(COL_TIME, "IR compilation (%s) took %" PRIu64" usec", cgm.getName().c_str(), t3b - t3a);

        if (!keep) cgm.remove_tmp();
        return true;
    }
private:
    void run() {
        while (1) {
            bool verbose = queue.getVerbose();
            bool printTiming = queue.getPrintTiming();
            bool keep_intermediates = queue.getKeepIntermediates();
            OptimizationLevel opt = queue.getOptimizationLevel();
            CodeGenModule* cgm = queue.get();
            if (!cgm) break;
            if (!build_module(*cgm, verbose, printTiming, keep_intermediates, opt)) {
                queue.fail();
                break;
            }
        }
    }
    static void* thread_main(void* arg) {
        IrWorker* worker = reinterpret_cast<IrWorker*>(arg);
        worker->run();
        return 0;
    }

    unsigned index;
    BuildQueue& queue;
    pthread_t thread;
};


IRGenerator::IRGenerator(const std::string& name_,
                         const std::string& dir_,
                         const ModuleList& modules_,
                         bool single_module_,
                         bool single_threaded_,
                         bool keep_intermediates_,
                         bool verbose_,
                         bool printTiming_,
                         bool printIR_,
                         bool useColors_)
    : name(name_)
    , dir(dir_)
    , modules(modules_)
    , single_module(single_module_)
    , single_threaded(single_threaded_)
    , keep_intermediates(keep_intermediates_)
    , verbose(verbose_)
    , printTiming(printTiming_)
    , printIR(printIR_)
    , useColors(useColors_)
{}

void IRGenerator::build() {
    StringList objects;

    OptimizationLevel opt = O2;
    if (single_module) {
        CodeGenModule cgm(name, dir, true, modules);
        objects.push_back(name);
        if (!IrWorker::build_module(cgm, verbose, printTiming, keep_intermediates, opt)) return;
    } else {
        uint64_t t1a = Utils::getCurrentTime();
        BuildQueue queue(verbose, printTiming, keep_intermediates, opt);
        for (unsigned m=0; m<modules.size(); m++) {
            Module* M = modules[m];
            if (M->isPlainC()) continue;
            if (M->getName() == "c2") continue;

            ModuleList single;
            single.push_back(M);
            CodeGenModule* cgm = new CodeGenModule(M->getName(), dir, false, single);
            queue.add(cgm);
            objects.push_back(M->getName());
        }

        unsigned num_threads = Utils::online_cpus();
        if (num_threads > queue.size()) num_threads = queue.size();
        if (single_threaded) num_threads = 1;
        if (verbose) Log::log(COL_VERBOSE, "building IR (%u tasks) with %u thread(s)", queue.size(), num_threads);
        IrWorker* workers[MAX_THREADS] = { 0 };
        for (unsigned i=0; i<num_threads; i++) {
            workers[i] = new IrWorker(i+1, queue);
        }

        for (unsigned i=0; i<num_threads; i++) {
            IrWorker* worker = workers[i];
            worker->join();
            delete worker;
        }
        uint64_t t1b = Utils::getCurrentTime();
        if (printTiming) Log::log(COL_TIME, "IR build took %" PRIu64" usec", t1b - t1a);
        if (queue.hasFailed()) return;
    }
    if (verbose) Log::log(COL_VERBOSE, "linking %s", name.c_str());
    uint64_t t4a = Utils::getCurrentTime();
    CodeGenModule::link(dir, name, objects);
    uint64_t t4b = Utils::getCurrentTime();
    if (printTiming) Log::log(COL_TIME, "linking took %" PRIu64" usec", t4b - t4a);
}

