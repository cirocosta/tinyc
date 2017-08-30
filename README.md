# tinyc

> A tiny "container" runtime


```

        mount           /bundle/rootfs  --> /


        chroot          provides the filesystem isolation
                        in a way that everything that the process sees
                        is a subdirectory of a real directory in the
                        parent fs.

                        /var/run/tinyc/containers/uuid/rootfs --> /


        clone           execute user process with a given set
                        of namespaces.


```


