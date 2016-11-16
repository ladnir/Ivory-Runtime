# The Ivory Secure Computation Runtime




<div style="float:right;width:50%;" align="left">
    <img src="icon.jpg" alt="Ivory Logo" style="width:304px;height:228px;">
    <div align="center"><font  color="#C8C8C8"> &copy;  2012-2016 WiorkaEG</font></div>
</div>


The Ivory runtime is a C++ library that aims to make secure computation easier to use. At a high level, Ivory acheives this by bringing together the protocol and the circuit compiler into a single integrated system. 

Consider the following code snippet. 

```c++
u32 program(CrtRemoteParty& them, CrtLocalParty& me, u32 myInput)
{
    // a public parameter determining the loop count
    u64 n = 100;

    // declaring private inputs, one for each party
    auto input0 = me.getIdx() == 0 ? me.input<CrtInt32>(myInput) : them.input<CrtInt32>();
    auto input1 = me.getIdx() == 1 ? me.input<CrtInt32>(myInput) : them.input<CrtInt32>();

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