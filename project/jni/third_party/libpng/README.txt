config-1.5.2.h and pnglibconf-1.5.2.h were generated using the following command:

PATH=$PATH:${NDK_HOME}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/darwin-x86/bin
P=${NDK_HOME}/platforms/android-9/arch-arm
CPPFLAGS=-I${P}/usr/include \
LDFLAGS=-L${P}/usr/lib\ -Wl,-rpath-link=${P}/usr/lib\ -nostdlib\ ${P}/usr/lib/crtbegin_static.o\ -lc \
./configure --host=arm-linux-androideabi --program-prefix=arm-linux-androideabi- --disable-shared --enable-static && \
make pnglibconf.h

