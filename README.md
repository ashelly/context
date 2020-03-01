# libContext
CONfiguration TEXT file parser

Format

    a_key a_value
    another_key value as multi word string
    "quoted key" "quoted value as single string"
    key
      nested_key1 value
      nested_key2 value2
    list [
       entry1_key1 value
       entry1_key2 value
    ] [
       entry2_key value
    ]
    

----
based on prior work
 - [khash](https://github.com/attractivechaos/klib)
 - [clibs/hash](https://github.com/clibs/hash)
 - [yaml](https://github.com/pbrandt1/yaml)
 - [sds](https://github.com/antirez/sds)
 - [LIST0](https://github.com/ashelly/LIST0)



