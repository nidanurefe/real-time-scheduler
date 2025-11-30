FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libqt5charts5-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build-cli -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build-cli --config Release

RUN cmake -S gui/rt_gui -B build-gui -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build-gui --config Release

WORKDIR /app

CMD ["./build-cli/rt_scheduler"]