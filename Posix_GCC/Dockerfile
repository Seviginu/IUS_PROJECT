# Базовый образ с Ubuntu
FROM ubuntu:20.04

# Установка часового пояса
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Moscow

# Установка необходимых инструментов
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gcc \
    gdb \
    git \
    tzdata \
    nano \
    && rm -rf /var/lib/apt/lists/* \
    && git clone --recurse-submodules https://github.com/FreeRTOS/FreeRTOS.git /opt/FreeRTOS \
    && mkdir -p /log \
    && rm -rf /opt/FreeRTOS/FreeRTOS/Demo/Posix_GCC

COPY . /opt/FreeRTOS/FreeRTOS/Demo/Posix_GCC

WORKDIR /opt/FreeRTOS/FreeRTOS/Demo/Posix_GCC

RUN make

CMD [ "/opt/FreeRTOS/FreeRTOS/Demo/Posix_GCC/build/posix_demo" ]


