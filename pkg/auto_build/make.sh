mkdir -p _bin
for package in tumatmul hey namaste
do
  make O=../../../trunk/obj/l4/x86/ -C $package
  mv $package/$package _bin/
done
