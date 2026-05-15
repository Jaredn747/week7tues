FROM ubuntu:24.04

RUN apt update && DEBIAN_FRONTEND=noninteractive TZ=US/LosAngeles apt install -y qemu-system-arm gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib git gdb-multiarch cmake

ENV VNC_RESOLUTION="1920x1200x16" \
    VNC_DPI="96"

ENTRYPOINT ["/bin/bash"]