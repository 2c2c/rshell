#rshell

### overview
`rshell` is an attempt at recreating a simple shell for unix based machines. In a nutshell this is just a parsing problem (i.e. figure out how to distinguish arbitrary operator tokens from the pile of commands you want to operate on, then use those operators correctly) combined with a unix syscalls problem (learning nuances of `fork` `exec` `wait` for processing commands and frivilous commands to pretty print your user info as you'd expect to see at a terminal).

### design

Two explicit ideas I had from the start were 1. Build the separation of stuff such that adding new operators is trivial. 2. use a flat coding style.

`1.` Is obvious. No one wants to rewrite half of their functions when the teacher asks to implement new stuff. The parsing I do takes any 'officially defined' single char operator from a special list and uses it as the means to treat that symbol as an operator. It handles n-size single character operator building pretty easily which is particularly usefor for this class given the operators we're being told to define.

`2.` Gave me problems. My initial thoughts were that there's pretty clearly no useful object structure, so working with free functions would be better. In the middle of coding I realized i was passing my input to just about every function in the program. Which is worse, a god class or a million functions being passed the same state?

### differences from bash

There's a couple of nuance differences from bash. 

Prompt line begins with `->` then `username@hostname ...` as normal. I was getting very annoyed at not being able to tell when I wasn't in rshell.

`ls & & ls` in bash will parse the &'s as two adjacent separate operators and give an error. `rshell` parses the command as `ls && ls` intentionally. When I delimit by the operators I purposely had spaces not be included in the list. This means that rather than getting a list like `"ls"->" "->"&"->" "->"&"->" "->"ls"`, `rshell` gets one like `"ls"->"&"->"&"->"ls"`. It's a little less work to just not handle the spaced out case, and one shouldn't be typing such silly input in the first place, so I chose to avoid it altogether.

Command interaction with operators is completely based on simple left to right parsing. `ls || date && pwd` will only print `ls` when bash would intelligently consider `ls || date` as 'successful' and exec `pwd` as a result

Comments were handled by deleting anything from `#` onwards regardless of how the symbol is situated in input. Meaning `asdf#asdf` in bash is interpreted as `asdf` in `rshell`.


### goodies

`rshell` doesn't break on adjacent implemented operators e.g. `ls && || ls` will properly output an error message

nor does it break with large amounts of the same operator e.g. `ls&&&&   ls`.

While these aren't particularly impressive feats, I am willing to guess that most other implementations won't actually handle the error. It may get indirectly handled by `execvp` while passing in some very strange input.
