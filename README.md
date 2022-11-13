# flyboy

![](https://www.theunderwoodfamily.net/public/flyboy.gif)

A simple Dreamcast game build for learning purposes only. It is complete, however, it has no difficulty curve.

Just needs docker to run and develop for from start to finish, but redream will only automatically install on linux. You'll have to figure out your own emulator or get it in a real Dreamcast otherwise.

```
git clone https://github.com/munderwoods/flyboy.git
cd flyboy
make init
make
```

The resulting test.cdi in the built-cdis directory will play on a Dreamcast like normal.

Technology used and code ripped off from:

- https://github.com/KallistiOS/KallistiOS
- https://gitlab.com/simulant/mkdcdisc
- https://github.com/Nold360/docker-kallistios-sdk
- https://github.com/ianmicheal/Dreamcast-ADX-homebrew-opensrc-
- https://github.com/DarkMorford/dreamcast101
