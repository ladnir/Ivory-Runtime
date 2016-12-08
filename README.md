# The Ivory Secure Computation Runtime




<div style="float:right;width:50%;" align="left">
    <img  align="right" src="icon.jpg" alt="Ivory Logo">
</div>


The Ivory Runtime is a C++ library that aims to make secure computation easier to use. At a high level, Ivory acheives this by bringing together the protocol and the binary/arithmetic circuit compiler into a single integrated system. 

Instead of requiring the user provide the circuit to be computed, the runtime pre-compiles many of the most useful opertions into mini-circuits/operations, e.g. addition, subtraction, multiplication, etc. The runtime then provides easy to use abstrations for declaring input variables, and computing with them. 

While at of this push, only semi-honest garbled circuit is supported, eventually other paradigms will be supported in a generic way. That is, you will be able to write a program that builds on Ivory's generic MPC API and then select the desired protocol to run in the background. E.g. semi-honest, malicious, garbled circuit, lego, mascot, etc...

Consider the following code snippet. It takes 64 bit input from two parties, multiplies them together and returns the result both parties. 

```c++
void program(std::array<Party, 2> parties, i64 myInput)
{
    // declare some secret inputs, one for each party
    sInt input0 = parties[0].isLocalParty() ?
        parties[0].input<sInt>(myInput, 64) :
        parties[0].input<sInt>(64);

    sInt input1 = parties[1].isLocalParty() ?
        parties[1].input<sInt>(myInput, 64) :
        parties[1].input<sInt>(64);

    // perform some generic secure computation 
    auto add = input1 + input0;
    auto sub = input1 - input0;
    auto mul = input1 * input0;
    auto div = input1 / input0;

    // logical operations
    auto gteq = input1 >= input0;
    auto lt   = input1 <  input0;

    // conditional operation
    auto max = gteq.ifelse(input1, input0);


    // mark these values as being revealed to party 0
    // at some point in the future (asychronous).
    parties[0].reveal(add);
    parties[0].reveal(sub);
    parties[0].reveal(mul);
    parties[0].reveal(div);

    // and these ones to party 1
    parties[1].reveal(gteq);
    parties[1].reveal(lt);
    parties[1].reveal(max);

    // The parties now waits for their results and prints them.
    if (parties[0].isLocalParty()) 
    {
        std::cout << "add  " << add.getValue() << std::endl;
        std::cout << "sub  " << sub.getValue() << std::endl;
        std::cout << "mul  " << mul.getValue() << std::endl;
        std::cout << "div  " << div.getValue() << std::endl;
    } else {
        std::cout << "gteq " << gteq.getValue() << std::endl;
        std::cout << "lt   " << lt.getValue() << std::endl;
        std::cout << "max  " << max.getValue() << std::endl;
    }

}
```