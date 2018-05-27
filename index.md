---
layout: default
---

# What is it?

Parlatype is a minimal audio player for manual speech transcription, written for the GNOME desktop environment. It plays audio sources to transcribe them in your favourite text application.

![Screenshot of main window](images/parlatype-main-window.png)

## Waveform
The waveform makes it easy to navigate in your audio file. You see what comes next and spot silence.

## Adjustable speed
You can control the speed of playback, playing slowly as you type, playing fast for editing. The pitch is not altered, there is no “chipmunk” effect.

## Rewind on pause
Whenever you pause playback, it will rewind a few seconds, so that you can easier resume. Of course you can change how much it rewinds or whether it rewinds at all.

## Timestamps
Parlatype produces timestamps which you can insert in your transcription. Parlatype will jump to that position at your will (drag 'n' drop or with LibreOffice Helpers).

## LibreOffice Helpers
Parlatype recommends to use LibreOffice. A set of helpers (macros) can be assigned to key bindings. E.g. insert timestamps or jump to timestamps.

## Plays almost every audio file
Parlatype is using the GStreamer framework which supports – with plugins – almost any audio file on your disk. Streaming media is not supported, you have to download it first.

## Media keys and foot pedals
Parlatype can be controlled with the “Play” button from your multimedia keyboard. This way it doesn’t have to have focus to control it. You can type in your text application and still have some (basic) control over Parlatype. Foot pedals can be assigned to the play button.

### Footpedals
This subsection documents how to get various footpedals to play nicely with Parlatype. Go to the [footpedals](footpedals/footpedals.md) section for details.

## Start on top
Parlatype can start on top of other windows. If you are working with a maximized text application, you can still see the Parlatype window.

## Help pages
Parlatype is easy to use nevertheless everything is documented in the help pages. You can see the [English help online](help-online/index.html).

## International
The user interface is fully translatable. Currently the program (without help pages) is available fully or almost fully translated in English, British English, Catalan, Dutch, French, German, Indonesian, Italian, Japanese, Lithuanian and Spanish. It's partly translated into Portuguese, Malay, Kurdish, Serbian, Kabyle and Latvian.

Any help in translations is welcome! Parlatype is translated at [https://translations.launchpad.net/parlatype](https://translations.launchpad.net/parlatype).

## For developers
Parlatype ships its own library, libparlatype, which provides a GStreamer backend (PtPlayer) and a waveviewer widget (PtWaveviewer) which is a GtkWidget. It is fully documented, however the API is not stable yet. See the [reference online](reference/html/index.html).

# Installation

## Flatpak
Parlatype is hosted on [Flathub](https://flathub.org/). Following the [instructions](https://flathub.org/#using) there you can install the [Parlatype package](https://flathub.org/apps.html#Parlatype). Please note that the LibreOffice helpers are not included in this package.

## Arch Linux

There is an AUR package for Parlatype: [https://aur.archlinux.org/packages/parlatype/](https://aur.archlinux.org/packages/parlatype/).

## Debian
Parlatype is in testing (Debian Buster), link to tracker: [https://tracker.debian.org/pkg/parlatype](https://tracker.debian.org/pkg/parlatype).

## Ubuntu
Parlatype is in Ubuntu's repository since 18.04 (Bionic Beaver).

There is a PPA with stable releases of Parlatype at [https://launchpad.net/~gabor-karsay/+archive/ubuntu/parlatype](https://launchpad.net/~gabor-karsay/+archive/ubuntu/parlatype).

## Other distros
You have to install Parlatype from source, see instructions on [https://github.com/gkarsay/parlatype](https://github.com/gkarsay/parlatype).

# Credits

The idea and initial design of Parlatype is based on Frederik Elwert’s program *transcribe*.

The display of the audio wave form started with code from *Buzztrax*.

The icon is based on work from the GNOME team.