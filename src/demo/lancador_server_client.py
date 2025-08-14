from subprocess import (Popen)
from pathlib import (Path)

TERMINAL = "/usr/bin/mate-terminal"
SERVER_EXE = Path("bin/demos/servidor_injetor")
CLIENT_EXE = Path("bin/demos/cliente_receptor")

def executa_o_programa(caminho: Path) -> Popen:
   return Popen([
      TERMINAL, "--window", '--command',
      caminho.absolute()
   ])


servidor = executa_o_programa(SERVER_EXE)
cliente  = executa_o_programa(CLIENT_EXE)

cliente.wait()
servidor.wait()

print(
   "Códigos de retorno: {} e {}"
   .format(cliente.returncode, servidor.returncode)
)
