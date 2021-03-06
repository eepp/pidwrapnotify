import notify2


def _send_pid_wrap_notification():
    body = 'Your system is starting a new round of PIDs.'
    n = notify2.Notification('PID wrapped!', body)
    n.show()


def _run_daemon():
    notify2.init('pidwrapnotifyd')

    with open('/dev/pidwrapnotify', 'rb', buffering=0) as f:
        while True:
            # wait for event
            f.read(0)

            # got it: notify
            _send_pid_wrap_notification()


def run():
    try:
        _run_daemon()
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    run()
