# NIOEchoServer

This sample application provides a simple echo server that sends clients back whatever data they send it. Invoke it using one of the following syntaxes:

```bash
language run NIOEchoServer  # Binds the server on ::1, port 9999.
language run NIOEchoServer 9899  # Binds the server on ::1, port 9899
language run NIOEchoServer /path/to/unix/socket  # Binds the server using the given UNIX socket
language run NIOEchoServer 192.168.0.5 9899  # Binds the server on 192.168.0.5:9899
```

