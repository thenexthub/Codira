# WebSocketServer

Class `WebSocketServer` represents:
* Server endpoint accepting new `WebSocket` connections,
* Single connection with client, encapsulated in `WebSocketBase` class.

All operations operations of the class (e.g. accepting new connection or receiving new messages) are blocking. Hence typical usage cases imply initializing server and starting a distinct _server thread_, which will:
* Call `AcceptNewConnection` when there is no active connection;
* Until the connection is ended in loop call `Decode`, handle the incoming message and reply with `SendReply`.

The connection might terminate either from client side or by call to `CloseConnection` (gracefully closes connection according to specification) or `Close` (will close both endpoint and active connection).

Interface is designed with trade safety, and all _external threads_ are able to call `SendReply`, `CloseConnection` and `Close`. Note that `SendReply` from an _external thread_ might happen concurrently with `CloseConnection` and `AcceptNewConnection`, so the message might be send to the new connection. To prevent such unexpected behavior, the _external thread_ can set callbacks to be triggered before the connection is finally closed, which is done with `SetCloseConnectionCallback` and `SetFailConnectionCallback` methods.

The following diagram sums up separation of concerns between server and external threads:

```mermaid
sequenceDiagram
    participant ExternalThread
    participant ServerThread
    participant WebSocketServer
    participant RemoteClient

    ExternalThread->>WebSocketServer: InitUnixWebSocket()
    ExternalThread->>ServerThread: spawn
    par While server thread running
        ServerThread->>WebSocketServer: AcceptNewConnection()
        RemoteClient->>WebSocketServer: accept()
        loop while connected
            ServerThread->>WebSocketServer: Decode()
            RemoteClient->>WebSocketServer: recv()
            alt Successful recv
                alt Close frame
                    WebSocketServer->>WebSocketServer: CloseConnection()
                else Data Frame
                    ServerThread->>ServerThread: DoHandlingLogic()
                    ServerThread->>WebSocketServer: SendReply()
                    WebSocketServer->>RemoteClient: send()
                end
            else Failed recv
                WebSocketServer->>WebSocketServer: CloseConnection()
            end
        end
    and While external thread running
        loop On event
            ExternalThread->>WebSocketServer: SendReply()
            WebSocketServer->>RemoteClient: send()
        end
        ExternalThread->>WebSocketServer: Close()
        WebSocketServer->>WebSocketServer: stop accepting, wait if active Connecting state
        WebSocketServer->>WebSocketServer: CloseConnection()
    end
```

## Connection

`WebSocketServer` incapsulates an active connection with a client. Methods `Decode`, `SendReply`, `CloseConnection` relate to a currently active connection (if any presents). Class maintains connection's state according to RFC6455:

```mermaid
stateDiagram-v2
    state Connecting
    state Open
    state Closing
    state Closed

    Closed --> Connecting : call AcceptNewConnection()
    Connecting --> Open : HttpHandShake() success
    Connecting --> Closed : HttpHandShake() failure
    Open --> Closing : call CloseConnection()
    Closing --> Closed : call CloseConnectionSocket()
```
