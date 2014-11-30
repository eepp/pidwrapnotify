import notify2


def _run_daemon():
    notify2.init('pidwrapnotifyd')

    with open('/dev/pidwrapnotify', 'rb', buffering=0) as f:
        while True:
            # wait for event
            f.read(0)

            # got it: notify
            body = 'Your system is starting a new round of PIDs.'
            n = notify2.Notification('PID wrapped!', body,
                                     'notification-message-im')
            n.show()


def run():
    try:
        _run_daemon()
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    run()
