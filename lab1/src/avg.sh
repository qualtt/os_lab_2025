sum=0
for i in "$@"
do
sum=$((sum+i))
done
average=$(echo "scale=2;$sum / $#" | bc)
echo "count: $#"
echo "average: $average"