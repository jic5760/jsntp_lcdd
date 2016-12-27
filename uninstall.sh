/bin/systemctl stop jsntp_lcdd.service
/bin/systemctl disable jsntp_lcdd.service
/bin/rm -f /lib/systemd/system/jsntp_lcdd.service
/usr/bin/id -u jsntpd >/dev/null &>/dev/null
rc=$?
if [ $rc -eq 0 ]; then
	/usr/sbin/userdel jsntpd
fi
exit 0

