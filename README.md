pidwrapnotify
=============

**pidwrapnotify** is a notifier of PID wrapping in Linux. A
kernel module notifies a user space daemon when the PID
wraps (that is, when a new process is created and its PID
is lower than the previous highest PID). The user space
daemon emits a D-Bus (standard) desktop notification when
this happens.


using
-----

This might not work with old Linux kernels.

  1. Clone the repository:

        git clone https://github.com/eepp/pidwrapnotify.git && cd pidwrapnotify

  2. Build the kernel module:

        ( cd module && make )

  3. Insert the kernel module into the Linux kernel:

        ( cd module && sudo insmod ./pidwrapnotify.ko )

  4. Look at the output of `dmesg` to find the character
     device's major number, e.g.:

        [ 5256.923685] pidwrapnotify: added char device 249:0

  5. Create the character special file using this major number:

        sudo mknod /dev/pidwrapnotify c 249 0

  6. Install the daemon (make sure you have the
     `python-dbus` package for Python 3):

        ( cd daemon && sudo ./setup.py install )

  7. Start the daemon:

        pidwrapnotifyd

  8. Enjoy.

Here's a simple way to make your system's PID wrap
quickly:

    while true; do ls; done

Of course, the actual time depends on this value:

    cat /proc/sys/kernel/pid_max
