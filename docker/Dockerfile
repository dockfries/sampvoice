FROM ubuntu:24.04
RUN \
    dpkg --add-architecture i386 && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        cmake \
        g++-multilib \
        libstdc++6:i386 \
        libc6:i386 \
    && \
    useradd -m user

USER user

ENV PATH=~/.local/bin:${PATH}

COPY docker-entrypoint.sh /
CMD /docker-entrypoint.sh