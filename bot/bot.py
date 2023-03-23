from socket import socket
from socket import AddressFamily, SocketKind
from deep_translator import GoogleTranslator, exceptions
import argparse
import sys

class Bot:

    __slots__ = ("_socket", "_host", "_port", "_user", "_nick", "_pswd", "_tr")

    def __init__(self, host, port, user, nick, pswd):
        self._host = host
        self._port = port
        self._user = user
        self._nick = nick
        self._pswd = pswd
        self._socket = socket(AddressFamily.AF_INET, SocketKind.SOCK_STREAM)

    @property
    def socket(self):
        return self._socket
    
    @socket.setter
    def socket(self, _socket : socket):
        self.socket = _socket
    
    @property
    def host(self):
        return self._host

    @property
    def port(self):
        return self._port

    @property
    def nick(self):
        return self._nick
    
    @property
    def user(self):
        return self._user

    @property
    def pswd(self):
        return self._pswd

    def identify(self):
        self.socket.send(bytes(f"PASS {self.pswd}\r\n", "utf-8"))
        self.socket.send(bytes(f"USER {self.user} {self.user} {self.user} :{self.user}\r\n", "utf-8"))
        self.socket.send(bytes(f"NICK {self.nick}\r\n", "utf-8"))
        responce, size = self.receive(4096)
        if "461" in responce or "431" in responce or "432" in responce or "433" in responce:
            print(responce.split('\r\n')[0])
            exit(1)

    def connect(self):
        self.socket.connect((self.host, self.port))
        self.identify()

    def receive(self, buff_size):
        response = self.socket.recv(buff_size).decode('utf-8')
        return response, len(response)

    def send(self, target, message):
        self.socket.send(bytes(f"PRIVMSG {target} :{message}\r\n", "utf-8"))

    def respond(self, request, size):
        args    = request.rsplit('\r\n', maxsplit = 1)[0]
        args    = args.split(maxsplit = 1)
        sender  = args[0]
        args    = args[-1].split(maxsplit = 1)
        command = args[0]
        args    = args[-1].split(':', maxsplit = 1)
        message = args[-1].strip() if len(args) == 2 else None

        if command == "PRIVMSG" and message:
            target = sender[1:].split('!')[0]
            tokens = [token.strip() for token in message.split('/') if token]
            text = tokens[-1]
            langs = tokens[0:-1]

            if len(langs) == 0:
                return
            elif len(langs) == 1:
                dstl, srcl = [langs[0]], "en"
            elif len(langs) > 1:
                dstl, srcl = langs[1:], langs[0]

            for dst in dstl:
                if dst == srcl:
                    continue
                try:
                    translated = GoogleTranslator(source=srcl, target=dst).translate(text=text)
                    self.send(target, f"[{dst}] {translated}")
                except exceptions.LanguageNotSupportedException:
                    self.send(target, f"[{dst}] No support for the provided language.")
                except Exception as E:
                    continue

    def run(self):
        self.connect()
        while True:
            request, size = self.receive(4096)
            if size == 0:
                print("Connection was closed.")
                break
            self.respond(request, size)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("name", type=str, default="TheBot", help="Username of the bot in the server")
    parser.add_argument("nick", type=str, default="BotNick", help="Nickname of the bot in the server")
    parser.add_argument("pswd", type=str, default="pass", help="Password of the server")
    arguments = parser.parse_args(sys.argv[1:])
    try:
        bot = Bot("localhost", 6667, arguments.name, arguments.nick, arguments.pswd)
        bot.run()
    except:
        exit(1)
