/* s390_table.h --
 * Copyright 2005-12 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *      Steve Grubb <sgrubb@redhat.com>
 */

_S(1, "exit")
_S(2, "fork")
_S(3, "read")
_S(4, "write")
_S(5, "open")
_S(6, "close")
_S(8, "creat")
_S(9, "link")
_S(10, "unlink")
_S(11, "execve")
_S(12, "chdir")
_S(13, "time")
_S(14, "mknod")
_S(15, "chmod")
_S(16, "lchown")
_S(19, "lseek")
_S(20, "getpid")
_S(21, "mount")
_S(22, "umount")
_S(23, "setuid")
_S(24, "getuid")
_S(25, "stime")
_S(26, "ptrace")
_S(27, "alarm")
_S(29, "pause")
_S(30, "utime")
_S(33, "access")
_S(34, "nice")
_S(36, "sync")
_S(37, "kill")
_S(38, "rename")
_S(39, "mkdir")
_S(40, "rmdir")
_S(41, "dup")
_S(42, "pipe")
_S(43, "times")
_S(45, "brk")
_S(46, "setgid")
_S(47, "getgid")
_S(48, "signal")
_S(49, "geteuid")
_S(50, "getegid")
_S(51, "acct")
_S(52, "umount2")
_S(54, "ioctl")
_S(55, "fcntl")
_S(57, "setpgid")
_S(60, "umask")
_S(61, "chroot")
_S(62, "ustat")
_S(63, "dup2")
_S(64, "getppid")
_S(65, "getpgrp")
_S(66, "setsid")
_S(67, "sigaction")
_S(70, "setreuid")
_S(71, "setregid")
_S(72, "sigsuspend")
_S(73, "sigpending")
_S(74, "sethostname")
_S(75, "setrlimit")
_S(76, "getrlimit")
_S(77, "getrusage")
_S(78, "gettimeofday")
_S(79, "settimeofday")
_S(80, "getgroups")
_S(81, "setgroups")
_S(83, "symlink")
_S(85, "readlink")
_S(86, "uselib")
_S(87, "swapon")
_S(88, "reboot")
_S(89, "readdir")
_S(90, "mmap")
_S(91, "munmap")
_S(92, "truncate")
_S(93, "ftruncate")
_S(94, "fchmod")
_S(95, "fchown")
_S(96, "getpriority")
_S(97, "setpriority")
_S(99, "statfs")
_S(100, "fstatfs")
_S(101, "ioperm")
_S(102, "socketcall")
_S(103, "syslog")
_S(104, "setitimer")
_S(105, "getitimer")
_S(106, "stat")
_S(107, "lstat")
_S(108, "fstat")
_S(111, "vhangup")
_S(112, "idle")
_S(114, "wait4")
_S(115, "swapoff")
_S(116, "sysinfo")
_S(117, "ipc")
_S(118, "fsync")
_S(119, "sigreturn")
_S(120, "clone")
_S(121, "setdomainname")
_S(122, "uname")
_S(124, "adjtimex")
_S(125, "mprotect")
_S(126, "sigprocmask")
_S(127, "create_module")
_S(128, "init_module")
_S(129, "delete_module")
_S(130, "get_kernel_syms")
_S(131, "quotactl")
_S(132, "getpgid")
_S(133, "fchdir")
_S(134, "bdflush")
_S(135, "sysfs")
_S(136, "personality")
_S(137, "afs_syscall")
_S(138, "setfsuid")
_S(139, "setfsgid")
_S(140, "_llseek")
_S(141, "getdents")
_S(142, "_newselect")
_S(143, "flock")
_S(144, "msync")
_S(145, "readv")
_S(146, "writev")
_S(147, "getsid")
_S(148, "fdatasync")
_S(149, "_sysctl")
_S(150, "mlock")
_S(151, "munlock")
_S(152, "mlockall")
_S(153, "munlockall")
_S(154, "sched_setparam")
_S(155, "sched_getparam")
_S(156, "sched_setscheduler")
_S(157, "sched_getscheduler")
_S(158, "sched_yield")
_S(159, "sched_get_priority_max")
_S(160, "sched_get_priority_min")
_S(161, "sched_rr_get_interval")
_S(162, "nanosleep")
_S(163, "mremap")
_S(164, "setresuid")
_S(165, "getresuid")
_S(167, "query_module")
_S(168, "poll")
_S(169, "nfsservctl")
_S(170, "setresgid")
_S(171, "getresgid")
_S(172, "prctl")
_S(173, "rt_sigreturn")
_S(174, "rt_sigaction")
_S(175, "rt_sigprocmask")
_S(176, "rt_sigpending")
_S(177, "rt_sigtimedwait")
_S(178, "rt_sigqueueinfo")
_S(179, "rt_sigsuspend")
_S(180, "pread")
_S(181, "pwrite")
_S(182, "chown")
_S(183, "getcwd")
_S(184, "capget")
_S(185, "capset")
_S(186, "sigaltstack")
_S(187, "sendfile")
_S(188, "getpmsg")
_S(189, "putpmsg")
_S(190, "vfork")
_S(191, "ugetrlimit")
_S(192, "mmap2")
_S(193, "truncate64")
_S(194, "ftruncate64")
_S(195, "stat64")
_S(196, "lstat64")
_S(197, "fstat64")
_S(198, "lchown32")
_S(199, "getuid32")
_S(200, "getgid32")
_S(201, "geteuid32")
_S(202, "getegid32")
_S(203, "setreuid32")
_S(204, "setregid32")
_S(205, "getgroups32")
_S(206, "setgroups32")
_S(207, "fchown32")
_S(208, "setresuid32")
_S(209, "getresuid32")
_S(210, "setresgid32")
_S(211, "getresgid32")
_S(212, "chown32")
_S(213, "setuid32")
_S(214, "setgid32")
_S(215, "setfsuid32")
_S(216, "setfsgid32")
_S(217, "pivot_root")
_S(218, "mincore")
_S(219, "madvise")
_S(220, "getdents64")
_S(221, "fcntl64")
_S(222, "readahead")
_S(223, "sendfile64")
_S(224, "setxattr")
_S(225, "lsetxattr")
_S(226, "fsetxattr")
_S(227, "getxattr")
_S(228, "lgetxattr")
_S(229, "fgetxattr")
_S(230, "listxattr")
_S(231, "llistxattr")
_S(232, "flistxattr")
_S(233, "removexattr")
_S(234, "lremovexattr")
_S(235, "fremovexattr")
_S(236, "gettid")
_S(237, "tkill")
_S(238, "futex")
_S(239, "sched_setaffinity")
_S(240, "sched_getaffinity")
_S(241, "tgkill")
//_S(242, "")
_S(243, "io_setup")
_S(244, "io_destroy")
_S(245, "io_getevents")
_S(246, "io_submit")
_S(247, "io_cancel")
_S(248, "exit_group")
_S(249, "epoll_create")
_S(250, "epoll_ctl")
_S(251, "epoll_wait")
_S(252, "set_tid_address")
_S(253, "fadvise64")
_S(254, "timer_create")
_S(255, "timer_settime")
_S(256, "timer_gettime")
_S(257, "timer_getoverrun")
_S(258, "timer_delete")
_S(259, "clock_settime")
_S(260, "clock_gettime")
_S(261, "clock_getres")
_S(262, "clock_nanosleep")
//_S(263, "")
_S(264, "fadvise64_64")
_S(265, "statfs64")
_S(266, "fstatfs64")
_S(267, "remap_file_pages")
//_S(268, "")
//_S(269, "")
//_S(270, "")
_S(271, "mq_open")
_S(272, "mq_unlink")
_S(273, "mq_timedsend")
_S(274, "mq_timedreceive")
_S(275, "mq_notify")
_S(276, "mq_getsetattr")
_S(277, "kexec_load")
_S(278, "add_key")
_S(279, "request_key")
_S(280, "keyctl")
_S(281, "waitid")
_S(282, "ioprio_set")
_S(283, "ioprio_get")
_S(284, "inotify_init")
_S(285, "inotify_add_watch")
_S(286, "inotify_rm_watch")
//_S(287, "")
_S(288, "openat")
_S(289, "mkdirat")
_S(290, "mknodat")
_S(291, "fchownat")
_S(292, "futimesat")
_S(293, "fstatat")
_S(294, "unlinkat")
_S(295, "renameat")
_S(296, "linkat")
_S(297, "symlinkat")
_S(298, "readlinkat")
_S(299, "fchmodat")
_S(300, "faccessat")
_S(301, "pselect6")
_S(302, "ppoll")
_S(303, "unshare")
_S(304, "set_robust_list")
_S(305, "get_robust_list")
_S(306, "splice")
_S(307, "sync_file_range")
_S(308, "tee")
_S(309, "vmsplice")
//_S(310, "")
_S(311, "getcpu")
_S(312, "epoll_pwait")
_S(313, "utimes")
_S(314, "fallocate")
_S(315, "utimensat")
_S(316, "signalfd")
_S(317, "timerfd")
_S(318, "eventfd")
_S(319, "timerfd_create")
_S(320, "timerfd_settime")
_S(321, "timerfd_gettime")
_S(322, "signalfd4")
_S(323, "eventfd2")
_S(324, "inotify_init1")
_S(325, "pipe2")
_S(326, "dup3")
_S(327, "epoll_create1")
_S(328, "preadv")
_S(329, "pwritev")
_S(330, "rt_tgsigqueueinfo")
_S(331, "perf_event_open")
_S(332, "fanotify_init")
_S(333, "fanotify_mark")
_S(334, "prlimit64")
_S(335, "name_to_handle_at")
_S(336, "open_by_handle_at")
_S(337, "clock_adjtime")
_S(338, "syncfs")
_S(339, "setns")
_S(340, "process_vm_readv")
_S(341, "process_vm_writev")
