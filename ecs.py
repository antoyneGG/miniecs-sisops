import socket
import sys

def create_instance( name ):
    sock = socket.create_connection(('localhost', 10000))
    print("Socked opened")
    try:
        petition = ("create " + name).encode()
        sock.sendall(petition)
    finally:
        print('Done!')
        sock.close()

def stop_instance( name ):
    sock = socket.create_connection(('localhost', 10000))
    print("Socked opened")
    try:
        petition = ("stop " + name).encode()
        sock.sendall(petition)
    finally:
        print('Done!')
        sock.close()

def delete_instance( name ):
    sock = socket.create_connection(('localhost', 10000))
    print("Socked opened")
    try:
        petition = ("delete " + name).encode()
        sock.sendall(petition)
    finally:
        print('Done!')
        sock.close()

def list_instances():
    sock = socket.create_connection(('localhost', 10000))
    print("Socked opened")
    try:
        petition = "list".encode()
        sock.sendall(petition)
    finally:
        print('Done!')
        sock.close()
