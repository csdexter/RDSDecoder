/* Arduino RDS/RDBS (IEC 62016/NRSC-4-B) Decoding Library
 * See the README file for author and licensing information. In case it's
 * missing from your distribution, use the one here as the authoritative
 * version: https://github.com/csdexter/RDSDecoder/blob/master/README
 *
 * This library is for decoding RDS/RBDS data streams (groups).
 *
 * This is an example sketch showing basic use of the library.
 * ----------------------------------------------------------------------------
 * NOTE: This library is meant to consume RDS group data obtained from
 *       somewhere else (e.g. an FM/RDS receiver chip), as such this example
 *       will not work as-is without modifications towards sourcing and
 *       providing such data to the library.
 *
 * NOTE: This library is meant as a toolbox for processing RDS data. RDS is a
 *       complex standard whose implementation details differ by locale and
 *       from radio station to radio station. As such, there is no "default
 *       way" of processing an incoming RDS data stream to output "the right
 *       data" with just a couple lines of code. You must be familiar with both
 *       the RDS standard and the target application (i.e. what data are you
 *       looking for, exactly) to be able to make effective use of the tools in
 *       this library.
 */

#include <RDSDecoder.h>

RDSDecoder the_decoder;
RDSTranslator the_translator;
word the_group[4]; // Put the externally sourced RDS data here
TRDSData the_data_so_far;

void OnEon(byte, bool, word, word) {
  // Do something with the received EON message, possibly using
  // RDSTranslator::decodeAFFrequency() to decode its contents.
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("RDS/RDBS Decoder Library Example"));
  the_decoder.registerCallback(RDS_CALLBACK_EON, OnEon);
}

void loop() {
  // Every time this is called 1) the internal data structure accessible via
  // RDSDecoder::getRDSData() is updated and 2) any relevant callbacks
  // registered via RDSDecoder::registerCallback() are called in turn.
  the_decoder.decodeRDSGroup(the_group);
  the_decoder.getRDSData(&the_data_so_far);
  Serial.print(F("PS: "));
  Serial.println(the_data_so_far.programService);
  Serial.print(F("RT: "));
  Serial.println(the_data_so_far.radioText);
}
