# tejoy
Крутой месенджер для крутого общения с крутыми людьми по крутому.
***
Месенджер использует P2P, и я не знаю что делать с NAT. Я бы мог сделать клиент-серверную архитектуру, но на сервер нет денег(На момент написания мне 12). NAT можно обойти используя Tailscale.

## Разработка в процесе
Месенджер пока ничего не может, я делаю отдельные модули

## Сборка
### Требования
* GCC
* Библиотека cJSON
* Библиотека sodium
* pkg-config
* Linux
* Make
* ~~Мозг~~
* Руки

### Установка зависимостей
#### Ubuntu/Debian/**Mint**
```bash
sudo apt update
sudo apt install build-essential pkg-config libcjson-dev libsodium-dev
```

#### CentOS/Fedora
```bash
sudo dnf update
sudo dnf install gcc make pkg-config libcjson-devel libsodium-devel # На счет `libcjson-devel` и `pkg-config` не уверен, может не скачается
```

#### Проверка установки
```bash
gcc --version
make --version
pkg-config --version
pkg-config --cflags --libs libcjson
pkg-config --cflags --libs libsodium
```

### Сборка Debug версии
```bash
make clean
make debug
```
Приложение будет в `./bin/debug/app`

### Сборка Release версии
```bash
make clean
make release
```
Приложение будет в `./bin/release/app`
