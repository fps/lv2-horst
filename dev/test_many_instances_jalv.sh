trap "kill 0" SIGINT

for n in {0..55}; do 
  script -c "jalv -n $n http://fps.io/plugins/state-variable-filter-v2" /dev/null &
done

while true; do
  sleep 1
done
