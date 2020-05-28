---
layout: post
title: <p style="text-align: center;">What Is Inside the 20 Newsgroups Dataset?<br />Human Readable Text?<br />Yes, but Not Only (Also a Roman Gazebo)</p>
---

What do we expect to find in an NLP dataset? Probably not a picture like this one!

![\_config.yml]({{ site.contenturl }}/secrets/images/ROMAN/ROMAN.jpg)

Actually, the picture looked like this:

```
begin 644 ROMAN.BMP
M0DT:S0@``````!H#```,````P`-8`@$`"``H,%4L*%DX2%G*E95(4'U9:8&%
M@?HT-%T,,$A`55U]?>K*=769QLY(1,Y9<77*RLJJUN(P/%UI@8D8&%E(3%D0
M/%VZNKI55?)U@<[*JJH42&U$55DH*'&RVOH857T0/&$X4%6-KL:JSOH@>;:)
MB>HT/%D42'%=7<Y$1'V9F?_*B8DL,%D858&=KO^-MKH<89&5H?_*:6DDA<HL
M,%W*OKX8584\2&%U<?I$0%DT-'$<896%H<X@;:7*G9V-I>)(684D>;8<89FA
MI:65NM9QA;(@;:H\2&G*?7UYB<8D>;IYD;8T1%G&QL9E=94HA<H01&504(59
M5?\H)%W*LK*VMK9978E]A?I,55DHA<XP,&'&^OI$2%F)D?(44'G*D9%]F:%E
M8?HP.%5(885,68FAMOIM>;+*<7'*QL:VXO\878V5G>Y`4%UA<84<:9TX1%T0
```

The image is a result of decoding. Ok, now let's go in order!
The 20 Newsgroups dataset is a well-known NLP dataset consisting of nearly 18,000 emails each of which relates to one of 20 topics. An email contains body text, a header, a footer, and maybe quotes. The dataset is used, for example in text classification, topic modeling.

So, the 20 Newsgroups is a natural language dataset. However, it turned out, that there is not only plain text inside but also encoded images and archives with source code! As a kind of attachment to the main message.

The thing came to the surface while developing a library for topic modeling TopicNet: some topics of models trained on the 20 Newsgroups dataset had strange top tokens (like AX, G9V, GIZ). Looking at top documents for these topics revealed that there is a whole series of more than 10 emails which contained just parts of the encoded gazebo image!  A bit of text analysis (Heaps' law plot) helped fo find a couple of other such secrets.

References
* The 20 Newsgroups dataset: http://qwone.com/~jason/20Newsgroups
* TopicNet library: https://github.com/machine-intelligence-laboratory/TopicNet
* Heap's law: https://en.wikipedia.org/wiki/Heaps%27_law
