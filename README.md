# UEFI in Userspace

This is an attempt to run UEFI executables in Linux userspace!
`uiu` loads the executable to memory and starts it with a custom System Table.
Because kernels often want to use privileged instructions, it is started in KVM.
Calls to Boot Services trap out of KVM for easier debugging.

ExitBootServices() is implemented with kexec.

When starting Linux with this, apparently it doesn't find its hardware after it
has called ExitBootServices(). TODO: debug this
