from subprocess import (Popen)

TERMINAL = "/usr/bin/mate-terminal"
SERVER_EXE = "bin/demos/servidor_injetor"
CLIENT_EXE ="bin/demos/cliente_receptor"
servidor = Popen([TERMINAL, "--window", "-e '{}'".format(SERVER_EXE)], shell=True)
cliente  = Popen([TERMINAL, "--window", "-e '{}'".format(CLIENT_EXE)], shell=True)

cliente.wait()
servidor.wait()
