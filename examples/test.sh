#!/bin/bash

# All temporary files are generated in the output directory
export session="output/session"
export stdout="output/stdout"
export stderr="output/stderr"
export actual="output/actual"
export sorted_in="output/sorted_in"
export sorted_out="output/sorted_out"
export empty="/dev/null"

rm -f $session

function run() {
    export code="$1"
    shift
    export target="$1"
    export cmd="output/$target/$target"
    shift
    echo "Compiling $target" >> $session
    c2c $target              >> $session
    echo "Running $cmd $* {" >> $session
    time ($cmd $*  >$stdout  2>$stderr) 2>> $session
    export status="$?"
    echo "    exit: $status, expected: $code" >> $session
    if [ $status != $code ] ; then
        echo "$cmd -> $status, expected $code"
    fi
    echo "}"            >> $session
    echo "output {"     >> $session
    cat      $stdout    >> $session
    echo "}"            >> $session
    echo "stderr {"     >> $session
    cat      $stderr    >> $session
    echo "}"            >> $session
}

function expect() {
    export stream="$1"
    export output="$2"
    if [ "$output" == "" ] ; then
        cat - > $actual
    elif [ "$output" == "==" ] ; then
        echo "$3" > $actual
    else
        cat $output > $actual
    fi
    diff -q $stream $actual
    if [ $? != 0 ] ; then
        echo "$cmd $stream output differs:"
        diff -u $stream $actual
    fi
}

function expect_sorted() {
    export stream="$1"
    sort -u $stream > $sorted_in
    sort -u - > $sorted_out
    diff -q $sorted_in $sorted_out
    if [ $? != 0 ] ; then
        echo "$cmd $stream sorted output differs: {"
        cat $stream
        echo "}"
    fi
}

run 0 base64
expect $stderr $empty
expect $stdout <<EOF
[pleasure.]
   encoded: cGxlYXN1cmUu
   decoded: pleasure.
[The quick brown fox jumped]
   encoded: VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wZWQ=
   decoded: The quick brown fox jumped
EOF

run 2 cstrip
expect $stderr == "usage: parser [c-file]"
expect $stdout $empty

run 0 dir_walker
expect $stderr $empty
# output is directory order dependend
#expect $stdout $empty

run 1 dir_walker non-existent
# output error may be system dependent
expect $stderr == "cannot open directory 'non-existent': No such file or directory"
expect $stdout == "non-existent/"

run 0 dir_walker common
expect $stderr $empty
# output is directory order dependend
expect_sorted $stdout <<EOF
common/
  common/file/
    common/file/reader.c2
    common/file/writer.c2
  common/logger.c2
  common/string_buffer.c2
  common/color.c2
EOF

#run 0 event

run 2 file_ops
expect $stderr == "usage: file_ops [file]"
expect $stdout $empty

run 1 file_ops non-existent
expect $stderr == "cannot load 'non-existent': No such file or directory"
expect $stdout $empty

run 0 file_ops file_ops/main.c2
expect $stderr $empty
expect $stdout file_ops/main.c2

run 0 inline_asm
expect $stderr $empty
# output is time dependend
#expect $stdout $empty

run 2 json_parser
expect $stderr == "usage: json_parser [json-file]"
expect $stdout $empty

## takes too long
run 0 jump
expect $stderr $empty
expect $stdout <<EOF
size=200
--- A ---
function2(A)
returning from function2(A)
--- B ---
function2(B)
returning from function2(B)
--- C ---
function2(C)
returning from function2(C)
--- D ---
function2(D)
coming from jump ONE, res=5
--- E ---
function2(E)
returning from function2(E)
--- F ---
function2(F)
comping from jump TWO, res=6
--- G ---
function2(G)
returning from function2(G)
EOF

run 0 list
expect $stderr $empty
expect $stdout <<EOF
this program orders a DAG by depth, using List
all:
 A     -1
  -> B
  -> C
  -> D
  -> F
 B     -1
  -> D
 C     -1
  -> B
  -> E
 D     -1
  -> E
 E     -1
 F     -1
 G     -1
Ordered list
all:
 G     -1
 A     0
 C     1
 F     1
 B     2
 D     3
 E     4
EOF

run 0 load_file
expect $stderr $empty
expect $stdout <<EOF
FILE1: size 257
00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 
10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 
20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 
30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 
40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 
50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F 
60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F 
70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F 
80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F 
90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F 
A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF 
B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF 
C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF 
D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF 
E0 E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF 
F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF 
00 
FILE2: size 74
This is a random
text file that is used

to show how to embed TEXT files

EOF

run 1 log
expect $stderr $empty
# output is time dependent
#expect $stdout $empty

#run 0 mc_receiver
#run 0 mc_sender
#run 0 plugin1
#run 0 plugin2

run 2 plugin_mgr
expect $stderr == "usage: plugin_mgr [filename]"
expect $stdout $empty

run 0 pthread_test
expect $stderr $empty
# output might be out of order
expect_sorted $stdout <<EOF
started thread1
started thread2
running1
running2
running2
running1
stopping threads
running2
quit thread2
running1
quit thread1
done
EOF

#run 0 output/signal_test/signal_test
#run 0 output/socket/socket

run 0 string
expect $stderr $empty
expect $stdout <<EOF
size = 28
data = 'hello this is a test!  xy123'
EOF

run 0 sudoku
expect $stderr $empty
expect $stdout <<EOF
usage: sudoku [puzzle]
  h4
  h5
  u3
  u5
EOF

#run 0 output/terminal/terminal

run 2 toml_parser
expect $stderr == "usage: toml_parser [toml-file]"
expect $stdout $empty

run 0 unit_test
expect $stderr $empty
expect $stdout <<EOF
TEST [1/8] mod1.test1 [FAIL]
  ERR: unit_test/mod1_test.c2:18  assertion failed, expected 7, got 6
TEST [2/8] mod1.test2 [FAIL]
  LOG: helper
  ERR: unit_test/mod1_test.c2:25  assertion failed, expected 'foo', got 'bar'
TEST [3/8] mod1.test3 [OK]
  LOG: helper
  LOG: foo bar faa
TEST [4/8] mod1.test5 [FAIL]
  ERR: unit_test/mod1_test.c2:43  expected 0x03 at offset 2, got 0x08
TEST [5/8] mod1.test6 [OK]
TEST [6/8] mod1.test7 [FAIL]
  ERR: unit_test/mod1_test.c2:54  assertion failed, expected nil, got 0x1
TEST [7/8] mod2.test1 [OK]
TEST [8/8] mod2.test2 [OK]
RESULT: 8 tests (4 ok, 4 failed, 0 skipped) ran in 0 ms
EOF

run 2 xml_parser
expect $stderr == "usage: xml_parser [xml-file] <verbose>"
expect $stdout $empty

run 2 yaml_parser
expect $stderr == "usage: yaml_parser [yaml-file]"
expect $stdout $empty

#run 0 lua_test
#expect $stderr $empty
#expect $stdout lua/stdout
