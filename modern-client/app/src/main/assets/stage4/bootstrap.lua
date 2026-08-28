local game = {
    stage = 4,
    package = "com.domtokima.paddvn",
    server = "http://192.168.8.59/pirate/public",
    next = "decrypt-and-load-original-op"
}

assert(game.stage == 4)
return "BOOTSTRAP OK / stage=" .. game.stage .. " / next=" .. game.next
