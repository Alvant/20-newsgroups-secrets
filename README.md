# 20-newsgroups-secrets

Twenty Newsgroups dataset — is a popular NLP dataset which consists of nearly 20.000 email text messages on 20 topics. 
Each message has a body, a header, a footer, and a timestamp.
The dataset can be used, for example, in experiments connected with text classification, clusterization, and in particular with topic modeling.

Documents of this text collection are mostly plain natural language text files, which contain nothing special.
However, it turns out that some of them may have really unique stuff inside.
For example, encoded .bmp images — email attachments which are actually a part of the text message.

In the repository there are just a couple interesting things found in the 20 Newsgroups dataset.

The [notebook](Basic-Study-of-the-20-Newsgroups-Dataset.ipynb) illustrates some basic study of the dataset (which actually helped to find one of the encoded pictures, and so drew attention to the search for other secrets in the dataset).


## References

### Data

* [20 Newsgroups site](http://qwone.com/~jason/20Newsgroups)
* [Description](https://scikit-learn.org/stable/datasets/index.html#newsgroups-dataset) of how to work with the dataset using Scikit-learn
* [Scikit-learn tutorial](https://scikit-learn.org/stable/tutorial/text_analytics/working_with_text_data.html) for the dataset

### Other

* [TopicNet library](https://github.com/machine-intelligence-laboratory/TopicNet) whose development triggered the whole thing and helped to find what is found
* [Uuencoding](https://en.wikipedia.org/wiki/Uuencoding) — encoding format used for attachments in the dataset


## Contributors (in Alphabetical Order)

* [Evgeny Egorov](https://github.com/Evgeny-Egorov-Projects)
* [Vasiliy Alekseev](https://github.com/Alvant)
* [Victor Bulatov](https://github.com/bt2901)
