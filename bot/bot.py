from socket import socket
from socket import AddressFamily, SocketKind
from time   import sleep
from deep_translator import GoogleTranslator, exceptions

class Bot:

    __slots__ = ("_socket", "_host", "_port", "_user", "_nick", "_pswd", "_tr")

    def __init__(self, host, port, user, nick, pswd):
        self._host = host
        self._port = port
        self._user = user
        self._nick = nick
        self._pswd = pswd
        self._socket = socket(AddressFamily.AF_INET, SocketKind.SOCK_STREAM)
        self._tr = GoogleTranslator()

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

    @property
    def tr(self):
        return self._tr

    def identify(self):
        self.socket.send(bytes(f"PASS {self.pswd}\r\n", "utf-8"))
        self.socket.send(bytes(f"USER {self.user} {self.user} {self.user} :{self.user}\r\n", "utf-8"))
        self.socket.send(bytes(f"NICK {self.nick}\r\n", "utf-8"))
        self.receive(4096)

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
        params  = args[0]

        if len(args) == 2:
            message = args[-1]
        else:
            message = None

        print(f"({sender})({command})({params})({message})")

        if command == "PING":
            pong = f"PONG {self.nick} {' '.join(args.split()[1:])}"
            self.socket.send(bytes(f"{pong}\r\n", "utf-8"))

        elif command == "PRIVMSG" and message:
            target = sender[1:].split('!')[0]
            tokens = [token.strip() for token in message.split('/') if token]
            text = tokens[-1]
            langs = tokens[0:-1]

            print(langs)
            if len(langs) == 0:
                return
            if len(langs) == 1:
                dstl, srcl = [langs[0]], "en"
            elif len(langs) > 1:
                dstl, srcl = langs[1:], langs[0]

            for dst in dstl:
                if dst == srcl:
                    continue
                try:
                    translated = GoogleTranslator(source=srcl, target=dst).translate(text=text)
                    self.send(target, f"{target}[{dst}] {translated}")
                except exceptions.LanguageNotSupportedException:
                    self.send(target, f"{target}[{dst}] No support for the provided language.")
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
    try:
        bot = Bot("localhost", 6667, "Bot", "Bot", "mok")
        bot.run()
    except:
        exit(1)
