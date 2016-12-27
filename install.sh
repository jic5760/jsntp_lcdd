/usr/bin/id -u jsntpd &>/dev/null
rc=$?
if [ ! $rc -eq 0 ]; then
	/usr/sbin/useradd -r jsntpd
	/usr/sbin/usermod -a -G i2c jsntpd
fi
/bin/cp -f Release/jsntp_lcdd /usr/local/bin/jsntp_lcdd
/bin/cp -f systemd/jsntp_lcdd.service /lib/systemd/system/jsntp_lcdd.service
/bin/systemctl daemon-reload
/bin/systemctl enable jsntp_lcdd.service
/bin/systemctl start jsntp_lcdd.service
exit 0

