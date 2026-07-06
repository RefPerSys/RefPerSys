ARG BASE_IMAGE=debian:sid-slim
FROM ${BASE_IMAGE}

ARG GCC_VERSION=15
ARG CLANG_VERSION=19
ARG LIGHTNING_VERSION=2.2.3
ARG LIBBACKTRACE_REF=master
ARG CARBURETTA_REF=master
ARG INSTALL_OPTIONAL_FONTS=1
ARG INSTALL_GUI_DEPS=0

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

ENV DEBIAN_FRONTEND=noninteractive
ENV GCC_VERSION=${GCC_VERSION}
ENV CLANG_VERSION=${CLANG_VERSION}
ENV CC=/usr/bin/gcc-${GCC_VERSION}
ENV CXX=/usr/bin/g++-${GCC_VERSION}
ENV CARBURETTA=/usr/local/bin/carburetta
ENV MAKE=/usr/local/bin/gmake
ENV RPS_LIBBACKTRACE=1
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
ENV LC_TIME=en_US.UTF-8
ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/share/pkgconfig
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV PATH=/usr/lib/ccache:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

RUN set -eux; \
    apt-get update; \
    apt-get dist-upgrade -y; \
    apt-get install -y --no-install-recommends \
      apt-utils \
      astyle \
      at \
      automake \
      autoconf \
      bash \
      bcpp \
      binutils-dev \
      bison \
      bisonc++ \
      bisonc++-doc \
      bsd-mailx \
      ca-certificates \
      ccache \
      clang-${CLANG_VERSION} \
      clang-tidy-${CLANG_VERSION} \
      cmake \
      curl \
      dpkg-dev \
      file \
      flex \
      g++-${GCC_VERSION} \
      gcc-${GCC_VERSION} \
      gdb \
      git \
      gpp \
      guile-3.0-dev \
      hostname \
      less \
      locales \
      libbz2-dev \
      libcurl4-openssl-dev \
      libcurlpp-dev \
      libgccjit-${GCC_VERSION}-dev \
      libgmp-dev \
      libc6-dev \
      libinih-dev \
      libjsoncpp-dev \
      libncurses-dev \
      libreadline-dev \
      libssl-dev \
      libtool \
      libunistring-dev \
      libzstd-dev \
      make-guile \
      msmtp-mta \
      ninja-build \
      pkg-config \
      plocate \
      procps \
      qt6-base-dev-tools \
      remake \
      tar \
      texinfo \
      uncrustify \
      xz-utils \
      zlib1g-dev; \
    ln -sf /usr/bin/make /usr/local/bin/gmake; \
    ln -sf /usr/bin/clang-tidy-${CLANG_VERSION} /usr/local/bin/clang-tidy; \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VERSION} 150; \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-${GCC_VERSION} 150; \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-${GCC_VERSION} 150; \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-${GCC_VERSION} 150; \
    sed -i 's/^# *\(en_US.UTF-8 UTF-8\)/\1/' /etc/locale.gen; \
    locale-gen en_US.UTF-8; \
    printf '/usr/local/lib\n' > /etc/ld.so.conf.d/usr-local-lib.conf; \
    rm -rf /var/lib/apt/lists/*

RUN set -eux; \
    if [[ "${INSTALL_OPTIONAL_FONTS}" == "1" ]]; then \
      apt-get update; \
      optional_fonts=( \
        fonts-cegui \
        fonts-croscore \
        fonts-dejavu \
        fonts-ecolier-court \
        fonts-eurofurence \
        fonts-inconsolata \
        fonts-inter \
        fonts-play \
        fonts-recommended \
        fonts-roboto \
        fonts-spleen \
        fonts-tuffy \
        fonts-ubuntu \
        fonts-unifont \
        fonts-yanone-kaffeesatz \
        msttcorefonts \
        ttf-mscorefonts-installer \
        ttf-unifont \
        unifont \
      ); \
      install_fonts=(); \
      for package in "${optional_fonts[@]}"; do \
        candidate="$(apt-cache policy "${package}" | sed -n 's/^  Candidate: //p' | head -n 1)"; \
        if [[ -n "${candidate}" && "${candidate}" != "(none)" ]]; then \
          install_fonts+=("${package}"); \
        fi; \
      done; \
      if (( ${#install_fonts[@]} )); then \
        apt-get install -y --no-install-recommends "${install_fonts[@]}"; \
      fi; \
      rm -rf /var/lib/apt/lists/*; \
    fi

RUN set -eux; \
    if [[ "${INSTALL_GUI_DEPS}" == "1" ]]; then \
      apt-get update; \
      optional_gui_deps=( \
        libfox-1.6-dev \
        libglibmm-2.68-dev \
        libgtkmm-3.0-dev \
        libgtkmm-4.0-dev \
        libonion-dev \
        qt6-base-dev \
      ); \
      install_gui_deps=(); \
      for package in "${optional_gui_deps[@]}"; do \
        if apt-cache show "${package}" >/dev/null 2>&1; then \
          install_gui_deps+=("${package}"); \
        fi; \
      done; \
      if (( ${#install_gui_deps[@]} )); then \
        apt-get install -y --no-install-recommends "${install_gui_deps[@]}"; \
      fi; \
      rm -rf /var/lib/apt/lists/*; \
    fi

RUN set -eux; \
    apt-get update; \
    apt-get install -y --no-install-recommends libiberty-dev; \
    rm -rf /var/lib/apt/lists/*

WORKDIR /tmp/refpersys-deps

RUN set -eux; \
    curl -fsSL "https://ftp.gnu.org/gnu/lightning/lightning-${LIGHTNING_VERSION}.tar.gz" -o lightning.tar.gz; \
    tar -xzf lightning.tar.gz; \
    cd "lightning-${LIGHTNING_VERSION}"; \
    ./configure \
      --with-gnu-ld \
      --enable-disassembler \
      --enable-devel-disassembler \
      --enable-devel-get-jit-size \
      --disable-silent-rules \
      CFLAGS='-O2 -g2'; \
    gmake -j"$(nproc)"; \
    gmake install; \
    ldconfig

RUN set -eux; \
    git clone --depth 1 https://github.com/ianlancetaylor/libbacktrace.git /tmp/refpersys-deps/libbacktrace; \
    cd /tmp/refpersys-deps/libbacktrace; \
    if [[ "${LIBBACKTRACE_REF}" != "master" ]]; then \
      git fetch --depth 1 origin "${LIBBACKTRACE_REF}"; \
      git checkout --detach FETCH_HEAD; \
    fi; \
    ./configure --prefix=/usr/local --enable-shared; \
    gmake -j"$(nproc)"; \
    gmake install; \
    ldconfig

RUN set -eux; \
    git clone --depth 1 https://github.com/kingletbv/carburetta.git /tmp/refpersys-deps/carburetta; \
    cd /tmp/refpersys-deps/carburetta; \
    if [[ "${CARBURETTA_REF}" != "master" ]]; then \
      git fetch --depth 1 origin "${CARBURETTA_REF}"; \
      git checkout --detach FETCH_HEAD; \
    fi; \
    gmake -j"$(nproc)" build/carburetta; \
    install -m 0755 build/carburetta /usr/local/bin/carburetta; \
    carburetta --help >/tmp/carburetta-help.txt; \
    grep -qi carburetta /tmp/carburetta-help.txt

RUN set -eux; \
    ldconfig; \
    updatedb; \
    test -x /usr/local/bin/gmake; \
    test -x /usr/local/bin/carburetta; \
    test -x /bin/at; \
    test -x /usr/lib/qt6/libexec/moc; \
    command -v getent; \
    command -v setpriv; \
    test -r /usr/local/include/lightning.h; \
    test -r /usr/local/include/backtrace.h; \
    test -r "$(${CC} -print-file-name=include)/libgccjit.h"; \
    command -v mail; \
    pkg-config --exists curlpp; \
    pkg-config --exists gmp; \
    pkg-config --exists gmpxx; \
    pkg-config --exists guile-3.0; \
    pkg-config --exists INIReader; \
    pkg-config --exists inih; \
    pkg-config --exists jsoncpp; \
    pkg-config --exists readline; \
    locale -a | grep -Ei '^en_US\.utf-?8$'; \
    locate libopcodes.so | grep -q .; \
    rm -rf /tmp/refpersys-deps

COPY docker/refpersys-build-entrypoint.sh /usr/local/bin/refpersys-build-entrypoint
RUN chmod +x /usr/local/bin/refpersys-build-entrypoint

WORKDIR /workspace/RefPerSys
ENTRYPOINT ["refpersys-build-entrypoint"]
