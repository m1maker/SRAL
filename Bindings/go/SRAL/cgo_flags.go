package SRAL

/*
#cgo CFLAGS: -I${SRCDIR}/../../../Include
#cgo darwin LDFLAGS: -L${SRCDIR}/lib -lSRAL -framework AppKit -framework Foundation -framework AVFoundation -lc++
#cgo linux pkg-config: speech-dispatcher
#cgo linux LDFLAGS: -L${SRCDIR}/lib -lSRAL -lbrlapi -lstdc++
#cgo windows CFLAGS: -DSRAL_STATIC
#cgo windows LDFLAGS: -L${SRCDIR}/lib -lSRAL -Wl,--start-group -luiautomationcore -lole32 -loleaut32 -luuid -luser32 -lkernel32 -lgdi32 -ladvapi32 -lshell32 -lstdc++ -Wl,--end-group -static
*/
import "C"
