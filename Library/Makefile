.PHONY: unpack-deps deps unpack-Klampt dep-Klampt unpack-lemon dep-lemon unpack-ompl dep-ompl

unpack-deps: unpack-Klampt unpack-lemon unpack-ompl

unpack-Klampt:
	git clone -b master https://github.com/RyodoTanaka/Klampt

unpack-lemon:
	wget http://lemon.cs.elte.hu/pub/sources/lemon-1.3.1.tar.gz
	tar xvzf lemon-1.3.1.tar.gz

unpack-ompl:
	git clone -b 1.4.0 https://github.com/RyodoTanaka/omplapp.git
	cd omplapp; git clone -b 1.4.0 https://github.com/RyodoTanaka/ompl.git

deps: dep-Klampt dep-lemon dep-ompl

dep-Klampt:
	cd Klampt/Library; make -j`nproc` unpack-deps; make -j`nproc` deps
	cd Klampt; cmake .; make -j`nproc`; sudo make -j`nproc` install

dep-lemon:
	cd lemon-1.3.1; mkdir build;
	cd lemon-1.3.1/build; cmake ..; make -j`nproc`; sudo make -j`nproc` install

dep-ompl:
	cd omplapp; mkdir -p build/Release;
	cd omplapp/build/Release; cmake ../..; make -j`nproc`; sudo make install
