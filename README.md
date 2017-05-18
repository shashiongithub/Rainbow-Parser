## MULTILINGUAL L-PCFG Rainbow Parser

# Models http://cohort.inf.ed.ac.uk/lpcfg.html



**Optimizing Spectral Learning for Parsing, Shashi Narayan and Shay
  B. Cohen, In the 54th Annual meeting of the Association for
  Computational Linguistics (ACL), Berlin 2016.**

> We describe a search algorithm for optimizing the number of latent
> states when estimating latent-variable PCFGs with spectral
> methods. Our results show that contrary to the common belief that the
> number of latent states for each nonterminal in an L-PCFG can be
> decided in isolation with spectral methods, parsing results
> significantly improve if the number of latent states for each
> nonterminal is globally optimized, while taking into account
> interactions between the different nonterminals.  In addition, we
> contribute an empirical analysis of spectral algorithms on eight
> morphologically rich languages: Basque, French, German, Hebrew,
> Hungarian, Korean, Polish and Swedish. Our results show that our
> estimation consistently performs better or close to coarse-to-fine
> expectation-maximization techniques for these languages.

**Diversity in Spectral Learning for Natural Language Parsing, Shashi
  Narayan and Shay B. Cohen, Empirical Methods on Natural Language Processing (EMNLP), Lisbon,
  Portugal 2015.**

> We describe an approach to create a diverse set of predictions with
> spectral learning of latent-variable PCFGs (L-PCFGs).  Our approach
> works by creating multiple spectral models where noise is added to the
> underlying features in the training set before the estimation of each
> model. We describe three ways to decode with multiple models. In
> addition, we describe a simple variant of the spectral algorithm for
> L-PCFGs that is fast and leads to compact models. Our experiments for
> natural language parsing, for English and German, show that we get a
> significant improvement over baselines comparable to state of the
> art. For English, we achieve the F1 score of 90.18, and for German we
> achieve the F1 score of 83.38.
       
# Java + Matlab code coming soon.
