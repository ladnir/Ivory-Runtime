# The Ivory Secure Computation Runtime




<div style="float:right;width:50%;" align="left">
    <img  align="right" src="icon.jpg" alt="Ivory Logo">
</div>


The Ivory Runtime is a C++ library that aims to make secure computation easier to use. At a high level, Ivory acheives this by bringing together the protocol and the circuit compiler into a single integrated system. 

Instead of requiring the user provide the circuit to be computed, the runtime pre-compiles many of the most useful opertions into mini-circuits, e.g. addition, subtraction, multiplication, etc. The runtime then provides easy to use abstrations for declaring input variables, and computing with them. 

While at of this push, only semi-honest garbled circuit addition/multion is supported, eventually other paradigms will be supported in a generic way. That is, you will be able to write a program that builds on Ivory's generic MPC API and then select the desired protocol to run in the background. E.g. semi-honest, malicious, garbled circuit, lego, mascot, etc...

Consider the following code snippet. It takes 32 bit input from two parties, multiplies them together and returns the result both parties. Depending on how `RemoteParty` and `LocalParty` were instantiated, different MPC protocol will perform the computation on the backend.

```c++
u32 program(RemoteParty& them, LocalParty& me, u32 myInput)
{
    // a public parameter determining the loop count
    u64 n = 100;

    // declaring private inputs, one for each party
    auto input0 = me.getIdx() == 0 ? me.input<sInt<32>>(myInput) : them.input<sInt<32>>();
    auto input1 = me.getIdx() == 1 ? me.input<sInt<32>>(myInput) : them.input<sInt<32>>();

    // perform the computation. Simply adding Party 0's input to Party 1's input
    // input to Party 2's input n times.
    for (u64 i = 0; i < n; ++i)
        input1 += input0;

    // the return is then decrypted, first to Party 0 and then to Party 1.
    u32 ret = 0;
    if (me.getIdx() == 0)
    {
        ret = me.reveal(input1);
        them.reveal(input1);
    }
    else
    {
        them.reveal(input1);
        ret = me.reveal(input1);
    }

    // Both Parties then return the result
    return ret;
}
```