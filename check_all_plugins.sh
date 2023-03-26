for n in `lv2ls`; do ./pywrap.sh check_uri.py "$n"; sleep 0.1 || break; done
