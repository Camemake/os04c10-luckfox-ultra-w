#!/usr/bin/env python3
import os
import socket
import threading
import time

latest = b""
lock = threading.Lock()
PAGE = b""
fps_text = b"0.0"
fps_count = 0
fps_t0 = time.time()


def reader():
    global latest, fps_count, fps_t0, fps_text
    buf = b""
    while True:
        chunk = os.read(0, 65536)
        if not chunk:
            os._exit(0)
        buf += chunk
        if len(buf) > 8000000:
            buf = buf[-2000000:]
        while True:
            start = buf.find(b"\xff\xd8")
            if start < 0:
                buf = buf[-1:]
                break
            end = buf.find(b"\xff\xd9", start + 2)
            if end < 0:
                if start:
                    buf = buf[start:]
                break
            frame = buf[start:end + 2]
            buf = buf[end + 2:]
            if len(frame) < 2000:
                continue
            with lock:
                latest = frame
            fps_count += 1
            now = time.time()
            if now - fps_t0 >= 2.0:
                fps_text = ("%.1f" % (fps_count / (now - fps_t0))).encode()
                fps_count = 0
                fps_t0 = now


def send_frame(client):
    frame = b""
    for _ in range(50):
        with lock:
            frame = latest
        if frame:
            break
        time.sleep(0.05)
    if not frame:
        client.sendall(b"HTTP/1.0 503 Busy\r\nContent-Length: 0\r\n\r\n")
        return
    client.sendall(
        b"HTTP/1.0 200 OK\r\n"
        b"Cache-Control: no-store\r\n"
        b"Content-Type: image/jpeg\r\n"
        b"Content-Length: %d\r\n\r\n" % len(frame) + frame
    )


def handle(client):
    try:
        client.settimeout(3)
        req = b""
        while b"\r\n\r\n" not in req and len(req) < 8192:
            chunk = client.recv(1024)
            if not chunk:
                return
            req += chunk
        line = req.split(b"\r\n", 1)[0]
        path = b"/"
        parts = line.split(b" ")
        if len(parts) >= 2:
            path = parts[1].split(b"?", 1)[0]
        client.settimeout(8)
        if path == b"/fps":
            body = fps_text + b"\n"
            client.sendall(
                b"HTTP/1.0 200 OK\r\n"
                b"Access-Control-Allow-Origin: *\r\n"
                b"Cache-Control: no-store\r\n"
                b"Content-Type: text/plain\r\n"
                b"Content-Length: %d\r\n\r\n" % len(body) + body
            )
            return
        if path in (b"/frame.jpg", b"/shot.jpg"):
            send_frame(client)
            return
        if path in (b"/", b"/live.html"):
            client.sendall(
                b"HTTP/1.0 200 OK\r\n"
                b"Cache-Control: no-store\r\n"
                b"Content-Type: text/html\r\n"
                b"Content-Length: %d\r\n\r\n" % len(PAGE) + PAGE
            )
            return
        client.sendall(
            b"HTTP/1.0 200 OK\r\n"
            b"Access-Control-Allow-Origin: *\r\n"
            b"Cache-Control: no-store\r\n"
            b"Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
            b"\r\n"
        )
        previous = b""
        while True:
            with lock:
                frame = latest
            if frame and frame != previous:
                header = (
                    "--frame\r\n"
                    "Content-Type: image/jpeg\r\n"
                    "Content-Length: %d\r\n\r\n" % len(frame)
                ).encode()
                client.sendall(header)
                client.sendall(frame)
                client.sendall(b"\r\n")
                previous = frame
            else:
                time.sleep(0.05)
    except Exception:
        pass
    finally:
        try:
            client.close()
        except Exception:
            pass


def main():
    global PAGE
    try:
        PAGE = open("/userdata/live.html", "rb").read()
    except OSError:
        PAGE = b"<p>no page</p>"
    threading.Thread(target=reader, daemon=True).start()
    server = socket.socket()
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", 8090))
    server.listen(8)
    while True:
        client, _addr = server.accept()
        threading.Thread(target=handle, args=(client,), daemon=True).start()


if __name__ == "__main__":
    main()
