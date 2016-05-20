# P4 Validation Model

This is a simple soft-switch that runs a P4 program and logs the output.
packets are modified sequentially in a single-threaded manner. Each packet
in the input will have a corresponding modified packet at the same position
in the output.

There is also a set of scripts and test files to allow validating the behaviour
of other models. The only requirement is that the model to be tested has the following
command line options:

```

-Xp4 <p4 program (json)>
-Xtpop <table population file>
-Xin <input pcap file>
-Xvalidation-out <output pcap file reordered according to input ordering>

```

The model can then be validated like so

```
./scripts/validate.sh path/to/model/executable

```

If necessary, it's also possible to fully specify the parameters for the comparison:

```
./scripts/validate.sh path/to/model/executable test-data/simple_router.json test-data/table.txt test-data/input.pcap

```
