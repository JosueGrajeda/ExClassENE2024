# invoke SourceDir generated makefile for release.pem4f
release.pem4f: .libraries,release.pem4f
.libraries,release.pem4f: package/cfg/release_pem4f.xdl
	$(MAKE) -f D:\PRACTI~4\u2\HVAC_UN_HILO_ORG\Aux_files/src/makefile.libs

clean::
	$(MAKE) -f D:\PRACTI~4\u2\HVAC_UN_HILO_ORG\Aux_files/src/makefile.libs clean

