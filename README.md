# xbee-posix-term
## POSIX Shell Terminal implementation over XBee

This is an application written to create an terminal-like application to send and receive shell commands over XBee while both nodes are connected to a POSIX-compliant device (e.g.: An embedded Linux device).
This code is strongly based on the original 'terminal_client' posix example from the Digi XBee ANSI C Library [here](https://github.com/digidotcom/xbee_ansic_library).

Application has been tested on an XBee PRO S3B 900HP module and can send simple terminal commands that don't depend on reading complex input data such as file contents for example.

### Instructions

The application take two arguments: (1) XBee module physical device path and (2) terminal mode.
1. Pass the module physical device path. e.g.: /dev/ttyUSB0
2. Pass either "host" or "client" to the second argument. Default mode is "host".
> - While in CLIENT mode, application reads a command and sends it to be executed on the remote node host system.
> - While in HOST mode, application sends received commands to the local terminal and sends the command output back to the remote node.

Command example: `./transparent_terminal /dev/ttyUSB0 client`

Before starting a terminal communication, you must set the targets on both ends.
You can either use the command `target`followed by the remote node NI value or set a manually mac address using the command mac, which will create a virtual NI based on the target node MAC address.

Example using target command (bold commands are typed by the user):
> <b>`target my_node`</b>

Example using mac command (bold commands are typed by the user):
><b>`nd`</b> (nd is the command for Node Discovey, in case nodes aren't scanned when application starts)

>`Address:0013a200-40eaeabc 0xfffe  PARENT:0xfffe  Router  NI:[]`

><b>`mac 0013a200-40eaeabc`</b>

>`Address:0013a200-40eaeabc 0xfffe  PARENT:0x0000   Coord  NI:[40EAEABC]`

><b>`target 40EAEABC`</b>

>`target: Address:0013a200-40eaeabc 0xfffe  PARENT:0x0000   Coord  NI:[40EAEABC]`

Type help from the application to get further assistance and more useful commands.

### Compilation

Project is built on GNU Make. Just run `make` to compile it. You must have the usual buildtools for your platform in order to compile.

### License

This project is distribuited under the GPL v3 license. Please read the [LICENSE](https://raw.githubusercontent.com/adfelippe/xbee-posix-term/master/LICENSE) file for further details.

### TO-DO

- Provide a tool to deal with complex commands which reads data from files such as `cat` or `dd`, for example.
- Write unit tests for current implementation
- Test on various embedded platforms to find any particularities.
