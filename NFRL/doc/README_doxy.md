## Configure

### Windows
See these Doxyfile (file) parameters:

```
PROJECT_NAME          = NFRL
HTML_EXTRA_STYLESHEET = nfrl.css
INPUT                 = ..\..\NFRL
OUTPUT_DIRECTORY      = ..\..\NFRL\doc
```

### Linux
Modify the Doxyfile parameters (above) to use forward-slash: `/`.

## Run

### Windows
The Doxyfile is configured to write output to `.\doc\html` dir.  It references the css file:  `.\doc\nfrl.css`.

```
.\NFRL\doc> "C:\Program Files\doxygen\bin\doxygen.exe"
```

### Linux
The Doxyfile is configured to write output to `./doc/html` dir.  It references the css file:  `./doc/nfrl.css`.

```
./NFRL/doc$ doxygen
```

## View in Browser
Navigate to file: `./doc/html/index.html` .
