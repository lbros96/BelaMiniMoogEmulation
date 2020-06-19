# BelaMiniMoogEmulation
A digital implementation of the Minimoog analog synthesizer with anti-aliased waveforms and a recreation of the moog ladder filter. Designed to be used with the Bela Platform (https://bela.io/).

Despite the ever increasing popularity of digital synthesis there is one criticism which constantly hangs over these methods. Namely, they fail to replicate some of the warmth and natural quality of their analog counterparts. There is a whole host of factors which contribute to the difference in the output of digital and analog synthesis methods. The discretization of time in the digital domain is one such example. Even the particular method of discretization will vary the resulting output significantly. The natural decay of the transistors over time and imperfect frequency response of the filters will also deviate the output signal from the "ideal" case but it is these deviations that both producers and listeners alike have grown to love. The stringency of code fails to capture these imperfections unless they are specifically accounted for. The goal of this project is to achieve a more realistic digital emulation of an analog synthesizer, in this case the Minimoog, with consideration for its analog specificities.

One of the key features of this emulation is the inclusion of quasi-band-limited waveforms using Polynomial Band-Limited Step Function proposed by Valimaki (2007). This method provides a computationally efficient way of reducing the aliasing in digital waveforms with discontinuities in the wave e.g. sawtooth and square waves.

This implementation also includes a digital recreation of the Moog ladder filter, now synonymous with the Moog sound. Recreating this digitally has proved to be a significant challenge as the its non-linear output proves difficult to model whilst maintaining filter stability. This digital realisation was based on an implementation suggested by Aaron Krajeski (2012).

# References
[1] Valimaki, V., & Huovilainen, A. (2007). Antialiasing Oscillators in Subtractive Synthesis. IEEE Signal Processing Magazine, 24(2), 116–125. doi:10.1109/msp.2007.323276

[2] Krajeski, A. (2012) http://song-swap.com/MUMT618/aaron/Presentation/contents.html
