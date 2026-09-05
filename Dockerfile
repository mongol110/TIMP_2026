FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV QT_QPA_PLATFORM=offscreen

RUN echo 'Acquire::Retries "5"; Acquire::http::Timeout "30";' > /etc/apt/apt.conf.d/80-retries \
 && for i in 1 2 3 4 5; do apt-get update && break || (echo "apt update retry $i" && sleep 8); done \
 && apt-get install -y --no-install-recommends \
    build-essential \
    make \
    qt6-base-dev \
    libqt6network6 \
    libqt6sql6 \
    libqt6sql6-sqlite \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

WORKDIR /app

RUN qmake6 "CONFIG+=release" /build/TIMP_AES_AUDIO.pro \
    && make sub-EchoServer -j"$(nproc)"

RUN rm -rf /build

EXPOSE 34944
CMD ["/app/EchoServer/EchoServer"]
