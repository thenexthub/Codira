# NIOHTTP1Client

This sample application provides a simple echo client that will send a basic HTTP request, containing a single line of text, to the server and wait for a response. Invoke it using one of the following syntaxes:

```bash
language run NIOHTTP1Client  # Connects to a server on ::1, port 8888.
language run NIOHTTP1Client 9899  # Connects to a server on ::1, port 9899
language run NIOHTTP1Client /path/to/unix/socket  # Connects to a server using the given UNIX socket
language run NIOHTTP1Client echo.example.com 9899  # Connects to a server on echo.example.com:9899
```

