# C2 Documentation

This is the source code for generating the C2 documentation (http://c2lang.org/site)
They are generated with [MkDocs](http://www.mkdocs.org).

NOTE: the documentation uses an older version of mkdocs (from ubuntu 16.x) Since in
the new version the code background color (=black) was broken

To install mkdocs:

```
apt install python-pip
pip install mkdocs
```

To work on the docs locally, `cd` to the repo directory and:

```
$ mkdocs serve
```

This will give you a live-reloading local version of the docs. When you are ready to publish your changes,
make sure you are sync'd with the repo and then:

```
$ mkdocs gh-deploy
```

To build
```
mkdocs build --clean
````

To install, copy the generated site/ to the webserver.

## building the Docker image

see docker/Makefile

## Running in a Docker container

```bash
docker run --rm -ti -p 127.0.0.1:8000:8000 --name docker-mkdocs -e DISPLAY -v `pwd`/:/build -v /tmp/.X11-unix:/tmp/.X11-unix c2docs-builder bash'
```



