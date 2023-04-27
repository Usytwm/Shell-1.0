# Shell-1.0

Este es un shell básico en C que permite al usuario ingresar comandos y ejecutarlos en un ambiente de línea de comandos. El shell es capaz de manejar comandos complejos con tuberías y redirecciones.

## Características

- Permite al usuario ingresar comandos y ejecutarlos en un ambiente de línea de comandos.
- Capacidad de manejar comandos complejos con tuberías y redirecciones.
- Almacena el historial de comandos ingresados por el usuario en un archivo.
- Ofrece autocompletado y sugerencias de comandos utilizando la biblioteca readline.
- Es compatible con la mayoría de los comandos y programas disponibles en la línea de comandos de Unix.

## Uso

Para ejecutar el shell, ejecuta el siguiente comando en la terminal:

```bash
make run
```

Una vez que el shell está en funcionamiento, el usuario puede ingresar comandos y ejecutarlos. Use las teclas de flecha hacia arriba y hacia abajo para navegar por el historial de comandos.

El shell admite los siguientes operadores:

- | - Tubería, permite redirigir la salida de un comando a la entrada de otro.
- &lt; - Redirecciona la entrada de un comando desde un archivo.

- &gt; - Redirecciona la salida de un comando a un archivo.

- &gt;&gt; - Anexa la salida de un comando a un archivo existente.

## Créditos

Este shell fue creado por Brian Ameht Inclan Quesada, Dariel Martinez Perez, Eric Lopez Tornas. Se utiliza la biblioteca readline para autocompletado y sugerencias de comandos.
