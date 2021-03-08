# the idea is this
the werckmeister midi player uses a javscript version of fluidsynth and therefore we use soundfonts.
To minimize the net traffic we compose the soundfont on the fly on client side.
To do so we download the needed samples before.

# sfsplit
this program splits a soundfont file into a skeleton, where all the sf header data is located, and into the sample files.

# sfcompose
this program is compiled via emscripten and runs on the client. It composes under the usage of the skeleton file and the sample data
a soundfont file only with the needed sample data.

# workflow
for example using the `FluidR3_GM` sounfont
## sfsplit
* create a output folder
* copy FluidR3_GM.sf2 into the folder
* execute `sfsplit $out/FluidR3_GM.sf2`

this will create a `FluidR3_GM.sf2.skeleton` file and ~1400 sample files: `FluidR3_GM.sf2.<sampleid>`

## sfcompose
### get the needed sample ids
* use sfcompose with the getsampleids command:
    * `sfcompose $pathToSkeleton --getsampleids [banknr presetnr]`
    * for example we want bank 0, preset 16 and bank 1, preset 5
    * `sfcompose out/FluidR3_GM.sf2.skeleton --getsampleids 0 16 1 5`
    * you get a list like this `0,1,2,4`
* use sfcompose to create the soundfont
    *  `sfcompose $pathToSkeleton $pathToSamples $samplePathTemplate $outfile [banknr presetnr]`
    * the `samplePathTemplate` means the part of any sample file before the number. So for samples like `FluidR3_GM.sf2.*.smpl` the `samplePathTemplate` is `FluidR3_GM.sf2.`
    * for example `sfcompose out/FluidR3_GM.sf2.skeleton out FluidR3_GM.sf2. mySoundfont.sf2 0 0 0 16`
# Sources
[polyphone](https://www.polyphone-soundfonts.com/)
[MuseScore](https://musescore.org/)