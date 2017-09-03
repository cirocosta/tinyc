# tinyc

`tinyc` is a tiny "container" runtime. 

It aims at containerizing process in the most lightway possible. If all you want is running a set of processes isolated from the others, `tinyc` may be handy for you.


## Usage

```
# run the 'sleep' process
tinyc sleep 33d  
```

```
# run the 'sleep' process with the root set to 'busybox'.
tinyc \
        --root=/var/lib/tinyc/images/busybox/rootfs \
        sleep 33d  
```

```
# run the 'sleep' process with a memory limit of 1M
tinyc \
        --memory=1M \
        sleep 33d  
```


```
# run the 'sleep' process with all privileges
tinyc \
        --privileged \
        sleep 33d  
```

```
# run the 'sleep' proccess that gets restarted in case of failures
tinyc \
        --restart=always \
        sleep 33d  
```


## Building

- `libcap-dev`
- `libseccomp-dev`



