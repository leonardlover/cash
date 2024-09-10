# cash
A simple Unix-like shell.

## Compilation

`cash` comes with a Makefile, so simply run

```bash
make
```

to compile the project.

## Execution

Run the following command to run `cash`.

```bash
./cash
```

Afterwards, cash will accept most Unix commands.

### Favoritos

There are some special commands which fall under the "Favoritos" categories, these are as follows:

`favs crear`: Will create a text file in the selected directory which will be used to store "favorite" commands. To run, type the following
```bash
favs crear /directory/ name.txt
```
Note that there must be a space between the last directory and the text file

`favs cargar`: Will load the "favorite" commands of the text file which name and directory matches the one given in `favs crear`. To run, type the following
```bash
favs cargar
```

`favs guardar`: Will save the "favorite" commands to the text file which name and directory matches the one given in `favs crear`. To run, type the following
```bash
favs cargar
```

`favs mostrar`: Will show the current "favorite" commands with their corresponding index number. To run, type the following
```bash
favs mostrar
```

`favs eliminar`: Will erase the "favorite" commands between the two given numbers. To run, type the following
```bash
favs eliminar num1, num2
```
The program saves the first two instances of concatenated numbers as `num1` and `num2` which are separated by any char that isn't a number. Additionaly, the two numbers don't have to be in order for the command to work.

`favs buscar`: Will search for "favorite" commands which have a given substring. To run, type the following
```bash
favs buscar cmd
```

`favs borrar`: Will erase the current "favorite" commands. To run, type the following
```bash
favs borrar
```

`favs ejecutar`: Will run a given command selected by their index. To run, type the following
```bash
favs num ejecutar
```
### Recordatorio
Another available command in `cash` is `set recordatorio`, which will create an alarm that will activate after a given ammount of seconds. A message will be displayed as well when this happens. To run, type the following:
```bash
set recordatorio time text
```

## Team
- Bastián Cabrera
- Leonardo Lovera
- José Cortés
- Ana Villagrán
