
<div class="PageDoc">

<div class="header">

<div class="headertitle">

<div class="title">

[VarioMS5611](classVarioMS5611.html "VarioMS5611 non-blocking data aquisition, for large OSR rates and accurate pressure,...")
library, supporting barometric variometer, altimeter,

</div>

</div>

</div>

<div class="contents">

<div class="textblock">

pressure & temperature provisioning in a accurate and smoothable manner

# <span id="intro_sec_de" class="anchor"></span> Summary

The
[VarioMS5611](classVarioMS5611.html "VarioMS5611 non-blocking data aquisition, for large OSR rates and accurate pressure,...")
library provides

  - access to the raw pressure and temperatur values of the MS5611
  - calculation of the effective pressure and temperature values by
    using the MS5611 internal factory calibration data
  - an interface to manage MS5611 internal oversampling rates (OSR)
  - an interface to manage smoothing factors for pressure and variometer
    values
  - an non blocking data aquisition method provided by using cooperative
    run() method, for sampling the pressure and temperature data
  - some extra methods to get statistical measure value information

# <span id="signal_sec" class="anchor"></span> Signal quality

Using this library a oversampling rate of about 160000 samples/second
can be reached. With according smoothing factors (\~0.93) a standard
deviation sigma of about 4-5cm/s can be reached. a signal-to-noise ratio
of about 20 for small climbing rates (70cm/s) and a signal noise about
less thn +-10cm/s see:

![Variometer-Plot](https://raw.githubusercontent.com/Pulsar07/VarioMS5611/master/doc/img/PlotOf-Vario-SigmaV-RelHeight.png)

# <span id="api_sec" class="anchor"></span> API

see: ![Class API
reference](https://htmlpreview.github.io/?https://github.com/Pulsar07/VarioMS5611/blob/master/doc/html/classVarioMS5611.html)

# <span id="hardware_sec" class="anchor"></span> Hardware

Specification of the MS5611/GY-63

  - <https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5611-01BA03%7FB3%7Fpdf%7FEnglish%7FENG_DS_MS5611-01BA03_B3.pdf>

</div>

</div>

</div>

-----

<span class="small">Generated
by [![doxygen](doxygen.svg)](http://www.doxygen.org/index.html)
1.8.20</span>
