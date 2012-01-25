/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "sstringparser.h"

namespace LXiCore {

QString SStringParser::iso639Language(const QString &lang)
{
  return iso639Language(lang.toAscii().data());
}

QString SStringParser::iso639Language(const char *lang)
{
  const int langLen = qstrnlen(lang, 4);

  if (langLen == 2)
  {
    for (const Iso639LangCode *i = iso639_1Codes(); i->code; i++)
    if (*reinterpret_cast<const quint16 *>(lang) == *reinterpret_cast<const quint16 *>(i->code))
      return QObject::tr(i->name);
  }
  else if (langLen == 3)
  {
    for (const Iso639LangCode *i = iso639_2Codes(); i->code; i++)
    if (*reinterpret_cast<const quint32 *>(lang) == *reinterpret_cast<const quint32 *>(i->code))
      return QObject::tr(i->name);
  }

  return lang;
}

QMap<QByteArray, QString> SStringParser::allIso639Languages(void)
{
  QMap<QByteArray, QString> result;
  for (const Iso639LangCode *i = iso639_2Codes(); i->code; i++)
    result.insert(i->code, QObject::tr(i->name));

  return result;
}

const char * SStringParser::codepageFor(const QString &lang)
{
  return codepageFor(lang.toAscii().data());
}

const char * SStringParser::codepageFor(const char *lang)
{
  if ((qstrcmp(lang, "cs") == 0) || (qstrcmp(lang, "ces") == 0) || (qstrcmp(lang, "cze") == 0) || // Czech
      (qstrcmp(lang, "hu") == 0) || (qstrcmp(lang, "hun") == 0) || // Hungarian
      (qstrcmp(lang, "pl") == 0) || (qstrcmp(lang, "pol") == 0) || // Polish
      (qstrcmp(lang, "ro") == 0) || (qstrcmp(lang, "ron") == 0) || (qstrcmp(lang, "rum") == 0) || // Romanian
      (qstrcmp(lang, "sk") == 0) || (qstrcmp(lang, "slk") == 0) || (qstrcmp(lang, "slo") == 0) || // Slovak
      (qstrcmp(lang, "sl") == 0) || (qstrcmp(lang, "slv") == 0)) // Slovenian
  {
    return "CP1250";
  }
  else if ((qstrcmp(lang, "bg") == 0) || (qstrcmp(lang, "bul") == 0) || // Russian
           (qstrcmp(lang, "ru") == 0) || (qstrcmp(lang, "rus") == 0))   // Bulgarian
  {
    return "CP1251";
  }
  else if ((qstrcmp(lang, "af") == 0) || (qstrcmp(lang, "afr") == 0) || // Afrikaans
           (qstrcmp(lang, "ca") == 0) || (qstrcmp(lang, "cat") == 0) || // Catalan
           (qstrcmp(lang, "da") == 0) || (qstrcmp(lang, "dan") == 0) || // Danish
           (qstrcmp(lang, "de") == 0) || (qstrcmp(lang, "ger") == 0) || // German
           (qstrcmp(lang, "en") == 0) || (qstrcmp(lang, "eng") == 0) || // English
           (qstrcmp(lang, "es") == 0) || (qstrcmp(lang, "esl") == 0) || (qstrcmp(lang, "spa") == 0) || // Spanish
           (qstrcmp(lang, "fi") == 0) || (qstrcmp(lang, "fin") == 0) || // Finnish
           (qstrcmp(lang, "fr") == 0) || (qstrcmp(lang, "fre") == 0) || (qstrcmp(lang, "fra") == 0) || // French
           (qstrcmp(lang, "fy") == 0) || (qstrcmp(lang, "fry") == 0) || // Frisian
           (qstrcmp(lang, "is") == 0) || (qstrcmp(lang, "ice") == 0) || (qstrcmp(lang, "isl") == 0) || // Icelandic
           (qstrcmp(lang, "it") == 0) || (qstrcmp(lang, "ita") == 0) || // Italian
           (qstrcmp(lang, "nl") == 0) || (qstrcmp(lang, "dut") == 0) || (qstrcmp(lang, "nla") == 0) || // Dutch
           (qstrcmp(lang, "no") == 0) || (qstrcmp(lang, "nor") == 0) || (qstrcmp(lang, "nno") == 0) || // Norwegian
           (qstrcmp(lang, "pt") == 0) || (qstrcmp(lang, "por") == 0) || // Portuguese
           (qstrcmp(lang, "sv") == 0) || (qstrcmp(lang, "swe") == 0) || (qstrcmp(lang, "sve") == 0)) // Swedish
  {
    return "CP1252";
  }
  else if ((qstrcmp(lang, "el") == 0) || (qstrcmp(lang, "ell") == 0) || (qstrcmp(lang, "gre") == 0)) // Greek
  {
    return "CP1253";
  }
  else if ((qstrcmp(lang, "tr") == 0) || (qstrcmp(lang, "tur") == 0)) // Turkish
  {
    return "CP1254";
  }
  else if ((qstrcmp(lang, "iw") == 0) || (qstrcmp(lang, "heb") == 0)) // Hebrew
  {
    return "CP1255";
  }
  else if ((qstrcmp(lang, "ar") == 0) || (qstrcmp(lang, "ara") == 0)) // Arabic
  {
    return "CP1256";
  }
  else if (qstrcmp(lang, "bat") == 0) // Baltic
  {
    return "CP1257";
  }
  else if ((qstrcmp(lang, "vi") == 0) || (qstrcmp(lang, "vie") == 0)) // Vietnamese
  {
    return "CP1258";
  }

  return NULL;
}

const SStringParser::LetterFrequency * SStringParser::letterFrequencies(void)
{
  static const LetterFrequency frequencies[] =
    { { "afr", L"eainrsotdlkgvmpubwfhjcêzëö" },
      { "cat", L"easirtocnlumdpgvfbqóàòhíèjwxyzl" },
      { "cze", L"oeantivlsrdkupímcházyjbřêéĉžýŝũgfúňwďóxťq" },
      { "dan", L"enadtrslgiohmkvufbpøjycéxqwèzüàóêç" },
      { "dut", L"enatirodslghvkmubpwjczfxyëéóq" },
      { "eng", L"etaoinsrhldcumfpgwybvkxjqz" },
      { "epo", L"aieonlsrtkjudmpvgfbcĝĉŭzŝhĵĥwyxq" },
      { "fin", L"enatrsildokgmvfuphäcböjyxzwq" },
      { "fre", L"esaitnrulodcmpévqfbghjàxèyêzçôùâûîœwkïëüæñ" },
      { "fry", L"erainftslbdkojgmuhypwczûâvúêôïëä" },
      { "ger", L"enisratdhulcgmobwfkzvüpäßjöyqx" },
      { "grc", L"αοιετσνηυρπκμλωδγχθφβξζψ" },
      { "heb", L"יהולארתבמשעםנכדחקפסזגצטןךףץ" },
      { "hun", L"eatlnskomzrigáéydbvhjfupöócíúüxwq" },
      { "ice", L"anriestudhlgmkfhvoáthídjóbyæúöpéýcxwzq" },
      { "ita", L"eaionlrtscdupmvghfbqzòàùìéèóykwxjô" },
      { "nor", L"erntsilakodgmvfupbhøjyæcwzxq" },
      { "pol", L"iaeoznscrwyłdkmtpujlgębąhżśóćńfźvqx" },
      { "por", L"aeosrinmdutlcphvqgfbãzçjáéxóõêôàyíèú" },
      { "ron", L"eaitnurcomăsdlpşvbţîfghzâjykxwq" },
      { "rus", L"oeaинтсвлркдмпуëягбзчйхжшюцщeф" },
      { "slo", L"aoesnitrvlkdmcupzyhjgfbqwx" },
      { "spa", L"eaosrnidlctumpbgyívqóhfzjéáñxúüwk" },
      { "swe", L"eantrslidomgkvähfupåöbcjyxwzéq" },
      { "tur", L"aeinrlıdkmuytsboüşzgçhğvcöpfjwxq" },
      { NULL, NULL } };

  return frequencies;
}

const SStringParser::Iso639LangCode * SStringParser::iso639_1Codes(void)
{
  static const Iso639LangCode codes[] =
      { { "aa", QT_TR_NOOP("Afar")           },
        { "ab", QT_TR_NOOP("Abkhazian")      },
        { "af", QT_TR_NOOP("Afrikaans")      },
        { "am", QT_TR_NOOP("Amharic")        },
        { "ar", QT_TR_NOOP("Arabic")         },
        { "as", QT_TR_NOOP("Assamese")       },
        { "ay", QT_TR_NOOP("Aymara")         },
        { "az", QT_TR_NOOP("Azerbaijani")    },
        { "ba", QT_TR_NOOP("Bashkir")        },
        { "be", QT_TR_NOOP("Byelorussian")   },
        { "bg", QT_TR_NOOP("Bulgarian")      },
        { "bh", QT_TR_NOOP("Bihari")         },
        { "bi", QT_TR_NOOP("Bislama")        },
        { "bn", QT_TR_NOOP("Bengali")        },
        { "bo", QT_TR_NOOP("Tibetan")        },
        { "br", QT_TR_NOOP("Breton")         },
        { "ca", QT_TR_NOOP("Catalan")        },
        { "co", QT_TR_NOOP("Corsican")       },
        { "cs", QT_TR_NOOP("Czech")          },
        { "cy", QT_TR_NOOP("Welsh")          },
        { "da", QT_TR_NOOP("Danish")         },
        { "de", QT_TR_NOOP("German")         },
        { "dz", QT_TR_NOOP("Bhutani")        },
        { "el", QT_TR_NOOP("Greek")          },
        { "en", QT_TR_NOOP("English")        },
        { "eo", QT_TR_NOOP("Esperanto")      },
        { "es", QT_TR_NOOP("Spanish")        },
        { "et", QT_TR_NOOP("Estonian")       },
        { "eu", QT_TR_NOOP("Basque")         },
        { "fa", QT_TR_NOOP("Persian")        },
        { "fi", QT_TR_NOOP("Finnish")        },
        { "fj", QT_TR_NOOP("Fiji")           },
        { "fo", QT_TR_NOOP("Faeroese")       },
        { "fr", QT_TR_NOOP("French")         },
        { "fy", QT_TR_NOOP("Frisian")        },
        { "ga", QT_TR_NOOP("Irish")          },
        { "gd", QT_TR_NOOP("Gaelic")         },
        { "gl", QT_TR_NOOP("Galician")       },
        { "gn", QT_TR_NOOP("Guarani")        },
        { "gu", QT_TR_NOOP("Gujarati")       },
        { "ha", QT_TR_NOOP("Hausa")          },
        { "hi", QT_TR_NOOP("Hindi")          },
        { "hr", QT_TR_NOOP("Croatian")       },
        { "hu", QT_TR_NOOP("Hungarian")      },
        { "hy", QT_TR_NOOP("Armenian")       },
        { "ia", QT_TR_NOOP("Interlingua")    },
        { "ie", QT_TR_NOOP("Interlingue")    },
        { "ik", QT_TR_NOOP("Inupiak")        },
        { "in", QT_TR_NOOP("Indonesian")     },
        { "is", QT_TR_NOOP("Icelandic")      },
        { "it", QT_TR_NOOP("Italian")        },
        { "iw", QT_TR_NOOP("Hebrew")         },
        { "ja", QT_TR_NOOP("Japanese")       },
        { "ji", QT_TR_NOOP("Yiddish")        },
        { "jw", QT_TR_NOOP("Javanese")       },
        { "ka", QT_TR_NOOP("Georgian")       },
        { "kk", QT_TR_NOOP("Kazakh")         },
        { "kl", QT_TR_NOOP("Greenlandic")    },
        { "km", QT_TR_NOOP("Cambodian")      },
        { "kn", QT_TR_NOOP("Kannada")        },
        { "ko", QT_TR_NOOP("Korean")         },
        { "ks", QT_TR_NOOP("Kashmiri")       },
        { "ku", QT_TR_NOOP("Kurdish")        },
        { "ky", QT_TR_NOOP("Kirghiz")        },
        { "la", QT_TR_NOOP("Latin")          },
        { "ln", QT_TR_NOOP("Lingala")        },
        { "lo", QT_TR_NOOP("Laothian")       },
        { "lt", QT_TR_NOOP("Lithuanian")     },
        { "lv", QT_TR_NOOP("Latvian")        },
        { "mg", QT_TR_NOOP("Malagasy")       },
        { "mi", QT_TR_NOOP("Maori")          },
        { "mk", QT_TR_NOOP("Macedonian")     },
        { "ml", QT_TR_NOOP("Malayalam")      },
        { "mn", QT_TR_NOOP("Mongolian")      },
        { "mo", QT_TR_NOOP("Moldavian")      },
        { "mr", QT_TR_NOOP("Marathi")        },
        { "ms", QT_TR_NOOP("Malay")          },
        { "mt", QT_TR_NOOP("Maltese")        },
        { "my", QT_TR_NOOP("Burmese")        },
        { "na", QT_TR_NOOP("Nauru")          },
        { "ne", QT_TR_NOOP("Nepali")         },
        { "nl", QT_TR_NOOP("Dutch")          },
        { "no", QT_TR_NOOP("Norwegian")      },
        { "oc", QT_TR_NOOP("Occitan")        },
        { "om", QT_TR_NOOP("Oromo")          },
        { "or", QT_TR_NOOP("Oriya")          },
        { "pa", QT_TR_NOOP("Punjabi")        },
        { "pl", QT_TR_NOOP("Polish")         },
        { "ps", QT_TR_NOOP("Pashto")         },
        { "pt", QT_TR_NOOP("Portuguese")     },
        { "qu", QT_TR_NOOP("Quechua")        },
        { "rm", QT_TR_NOOP("Rhaeto-Romance") },
        { "rn", QT_TR_NOOP("Kirundi")        },
        { "ro", QT_TR_NOOP("Romanian")       },
        { "ru", QT_TR_NOOP("Russian")        },
        { "rw", QT_TR_NOOP("Kinyarwanda")    },
        { "sa", QT_TR_NOOP("Sanskrit")       },
        { "sd", QT_TR_NOOP("Sindhi")         },
        { "sg", QT_TR_NOOP("Sangro")         },
        { "sh", QT_TR_NOOP("Serbo-Croatian") },
        { "si", QT_TR_NOOP("Singhalese")     },
        { "sk", QT_TR_NOOP("Slovak")         },
        { "sl", QT_TR_NOOP("Slovenian")      },
        { "sm", QT_TR_NOOP("Samoan")         },
        { "sn", QT_TR_NOOP("Shona")          },
        { "so", QT_TR_NOOP("Somali")         },
        { "sq", QT_TR_NOOP("Albanian")       },
        { "sr", QT_TR_NOOP("Serbian")        },
        { "ss", QT_TR_NOOP("Siswati")        },
        { "st", QT_TR_NOOP("Sesotho")        },
        { "su", QT_TR_NOOP("Sudanese")       },
        { "sv", QT_TR_NOOP("Swedish")        },
        { "sw", QT_TR_NOOP("Swahili")        },
        { "ta", QT_TR_NOOP("Tamil")          },
        { "te", QT_TR_NOOP("Tegulu")         },
        { "tg", QT_TR_NOOP("Tajik")          },
        { "th", QT_TR_NOOP("Thai")           },
        { "ti", QT_TR_NOOP("Tigrinya")       },
        { "tk", QT_TR_NOOP("Turkmen")        },
        { "tl", QT_TR_NOOP("Tagalog")        },
        { "tn", QT_TR_NOOP("Setswana")       },
        { "to", QT_TR_NOOP("Tonga")          },
        { "tr", QT_TR_NOOP("Turkish")        },
        { "ts", QT_TR_NOOP("Tsonga")         },
        { "tt", QT_TR_NOOP("Tatar")          },
        { "tw", QT_TR_NOOP("Twi")            },
        { "uk", QT_TR_NOOP("Ukrainian")      },
        { "ur", QT_TR_NOOP("Urdu")           },
        { "uz", QT_TR_NOOP("Uzbek")          },
        { "vi", QT_TR_NOOP("Vietnamese")     },
        { "vo", QT_TR_NOOP("Volapuk")        },
        { "wo", QT_TR_NOOP("Wolof")          },
        { "xh", QT_TR_NOOP("Xhosa")          },
        { "yo", QT_TR_NOOP("Yoruba")         },
        { "zh", QT_TR_NOOP("Chinese")        },
        { "zu", QT_TR_NOOP("Zulu")           },
        { NULL, NULL } };

  return codes;
}

const SStringParser::Iso639LangCode * SStringParser::iso639_2Codes(void)
{
  static const Iso639LangCode codes[] =
      { { "abk", QT_TR_NOOP("Abkhazian")                                                   	},
        { "ace", QT_TR_NOOP("Achinese")                                                     },
        { "ach", QT_TR_NOOP("Acoli")                                                        },
        { "ada", QT_TR_NOOP("Adangme")                                                      },
        { "aar", QT_TR_NOOP("Afar")                                                         },
        { "afh", QT_TR_NOOP("Afrihili")                                                     },
        { "afr", QT_TR_NOOP("Afrikaans")                                                    },
        { "afa", QT_TR_NOOP("Afro-Asiatic (Other)")                                         },
        { "aka", QT_TR_NOOP("Akan")                                                         },
        { "akk", QT_TR_NOOP("Akkadian")                                                     },
        { "alb", QT_TR_NOOP("Albanian")                                                     },
        { "sqi", QT_TR_NOOP("Albanian")                                                     },
        { "ale", QT_TR_NOOP("Aleut")                                                        },
        { "alg", QT_TR_NOOP("Algonquian languages")                                         },
        { "tut", QT_TR_NOOP("Altaic (Other)")                                               },
        { "amh", QT_TR_NOOP("Amharic")                                                      },
        { "apa", QT_TR_NOOP("Apache languages")                                             },
        { "ara", QT_TR_NOOP("Arabic")                                                       },
        { "arc", QT_TR_NOOP("Aramaic")                                                      },
        { "arp", QT_TR_NOOP("Arapaho")                                                      },
        { "arn", QT_TR_NOOP("Araucanian")                                                   },
        { "arw", QT_TR_NOOP("Arawak")                                                       },
        { "arm", QT_TR_NOOP("Armenian")                                                     },
        { "hye", QT_TR_NOOP("Armenian")                                                     },
        { "art", QT_TR_NOOP("Artificial (Other)")                                           },
        { "asm", QT_TR_NOOP("Assamese")                                                     },
        { "ath", QT_TR_NOOP("Athapascan languages")                                         },
        { "map", QT_TR_NOOP("Austronesian (Other)")                                         },
        { "ava", QT_TR_NOOP("Avaric")                                                       },
        { "ave", QT_TR_NOOP("Avestan")                                                      },
        { "awa", QT_TR_NOOP("Awadhi")                                                       },
        { "aym", QT_TR_NOOP("Aymara")                                                       },
        { "aze", QT_TR_NOOP("Azerbaijani")                                                  },
        { "nah", QT_TR_NOOP("Aztec")                                                        },
        { "ban", QT_TR_NOOP("Balinese")                                                     },
        { "bat", QT_TR_NOOP("Baltic (Other)")                                               },
        { "bal", QT_TR_NOOP("Baluchi")                                                      },
        { "bam", QT_TR_NOOP("Bambara")                                                      },
        { "bai", QT_TR_NOOP("Bamileke languages")                                           },
        { "bad", QT_TR_NOOP("Banda")                                                        },
        { "bnt", QT_TR_NOOP("Bantu (Other)")                                                },
        { "bas", QT_TR_NOOP("Basa")                                                         },
        { "bak", QT_TR_NOOP("Bashkir")                                                      },
        { "baq", QT_TR_NOOP("Basque")                                                       },
        { "eus", QT_TR_NOOP("Basque")                                                       },
        { "bej", QT_TR_NOOP("Beja")                                                         },
        { "bem", QT_TR_NOOP("Bemba")                                                        },
        { "ben", QT_TR_NOOP("Bengali")                                                      },
        { "ber", QT_TR_NOOP("Berber (Other)")                                               },
        { "bho", QT_TR_NOOP("Bhojpuri")                                                     },
        { "bih", QT_TR_NOOP("Bihari")                                                       },
        { "bik", QT_TR_NOOP("Bikol")                                                        },
        { "bin", QT_TR_NOOP("Bini")                                                         },
        { "bis", QT_TR_NOOP("Bislama")                                                      },
        { "bra", QT_TR_NOOP("Braj")                                                         },
        { "bre", QT_TR_NOOP("Breton")                                                       },
        { "bug", QT_TR_NOOP("Buginese")                                                     },
        { "bul", QT_TR_NOOP("Bulgarian")                                                    },
        { "bua", QT_TR_NOOP("Buriat")                                                       },
        { "bur", QT_TR_NOOP("Burmese")                                                      },
        { "mya", QT_TR_NOOP("Burmese")                                                      },
        { "bel", QT_TR_NOOP("Byelorussian")                                                 },
        { "cad", QT_TR_NOOP("Caddo")                                                        },
        { "car", QT_TR_NOOP("Carib")                                                        },
        { "cat", QT_TR_NOOP("Catalan")                                                      },
        { "cau", QT_TR_NOOP("Caucasian (Other)")                                            },
        { "ceb", QT_TR_NOOP("Cebuano")                                                      },
        { "cel", QT_TR_NOOP("Celtic (Other)")                                               },
        { "cai", QT_TR_NOOP("Central American Indian (Other)")                              },
        { "chg", QT_TR_NOOP("Chagatai")                                                     },
        { "cha", QT_TR_NOOP("Chamorro")                                                     },
        { "che", QT_TR_NOOP("Chechen")                                                      },
        { "chr", QT_TR_NOOP("Cherokee")                                                     },
        { "chy", QT_TR_NOOP("Cheyenne")                                                     },
        { "chb", QT_TR_NOOP("Chibcha")                                                      },
        { "chi", QT_TR_NOOP("Chinese")                                                      },
        { "zho", QT_TR_NOOP("Chinese")                                                      },
        { "chn", QT_TR_NOOP("Chinook jargon")                                               },
        { "cho", QT_TR_NOOP("Choctaw")                                                      },
        { "chu", QT_TR_NOOP("Church Slavic")                                                },
        { "chv", QT_TR_NOOP("Chuvash")                                                      },
        { "cop", QT_TR_NOOP("Coptic")                                                       },
        { "cor", QT_TR_NOOP("Cornish")                                                      },
        { "cos", QT_TR_NOOP("Corsican")                                                     },
        { "cre", QT_TR_NOOP("Cree")                                                         },
        { "mus", QT_TR_NOOP("Creek")                                                        },
        { "crp", QT_TR_NOOP("Creoles and Pidgins (Other)")                                  },
        { "cpe", QT_TR_NOOP("Creoles and Pidgins, English-based (Other)")                   },
        { "cpf", QT_TR_NOOP("Creoles and Pidgins, French-based (Other)")                    },
        { "cpp", QT_TR_NOOP("Creoles and Pidgins, Portuguese-based (Other)")                },
        { "cus", QT_TR_NOOP("Cushitic (Other)")                                             },
        { "ces", QT_TR_NOOP("Czech")                                                        },
        { "cze", QT_TR_NOOP("Czech")                                                        },
        { "dak", QT_TR_NOOP("Dakota")                                                       },
        { "dan", QT_TR_NOOP("Danish")                                                       },
        { "del", QT_TR_NOOP("Delaware")                                                     },
        { "din", QT_TR_NOOP("Dinka")                                                        },
        { "div", QT_TR_NOOP("Divehi")                                                       },
        { "doi", QT_TR_NOOP("Dogri")                                                        },
        { "dra", QT_TR_NOOP("Dravidian (Other)")                                            },
        { "dua", QT_TR_NOOP("Duala")                                                        },
        { "dut", QT_TR_NOOP("Dutch")                                                        },
        { "nla", QT_TR_NOOP("Dutch")                                                        },
        { "dum", QT_TR_NOOP("Dutch, Middle (ca. 1050-1350)")                                },
        { "dyu", QT_TR_NOOP("Dyula")                                                        },
        { "dzo", QT_TR_NOOP("Dzongkha")                                                     },
        { "efi", QT_TR_NOOP("Efik")                                                         },
        { "egy", QT_TR_NOOP("Egyptian (Ancient)")                                           },
        { "eka", QT_TR_NOOP("Ekajuk")                                                       },
        { "elx", QT_TR_NOOP("Elamite")                                                      },
        { "eng", QT_TR_NOOP("English")                                                      },
        { "enm", QT_TR_NOOP("English, Middle (ca. 1100-1500)")                              },
        { "ang", QT_TR_NOOP("English, Old (ca. 450-1100)")                                  },
        { "esk", QT_TR_NOOP("Eskimo (Other)")                                               },
        { "epo", QT_TR_NOOP("Esperanto")                                                    },
        { "est", QT_TR_NOOP("Estonian")                                                     },
        { "ewe", QT_TR_NOOP("Ewe")                                                          },
        { "ewo", QT_TR_NOOP("Ewondo")                                                       },
        { "fan", QT_TR_NOOP("Fang")                                                         },
        { "fat", QT_TR_NOOP("Fanti")                                                        },
        { "fao", QT_TR_NOOP("Faroese")                                                      },
        { "fij", QT_TR_NOOP("Fijian")                                                       },
        { "fin", QT_TR_NOOP("Finnish")                                                      },
        { "fiu", QT_TR_NOOP("Finno-Ugrian (Other)")                                         },
        { "fon", QT_TR_NOOP("Fon")                                                          },
        { "fra", QT_TR_NOOP("French")                                                       },
        { "fre", QT_TR_NOOP("French")                                                       },
        { "frm", QT_TR_NOOP("French, Middle (ca. 1400-1600)")                               },
        { "fro", QT_TR_NOOP("French, Old (842- ca. 1400)")                                  },
        { "fry", QT_TR_NOOP("Frisian")                                                      },
        { "ful", QT_TR_NOOP("Fulah")                                                        },
        { "gaa", QT_TR_NOOP("Ga")                                                           },
        { "gae", QT_TR_NOOP("Gaelic (Scots)")                                               },
        { "gdh", QT_TR_NOOP("Gaelic (Scots)")                                               },
        { "glg", QT_TR_NOOP("Gallegan")                                                     },
        { "lug", QT_TR_NOOP("Ganda")                                                        },
        { "gay", QT_TR_NOOP("Gayo")                                                         },
        { "gez", QT_TR_NOOP("Geez")                                                         },
        { "geo", QT_TR_NOOP("Georgian")                                                     },
        { "kat", QT_TR_NOOP("Georgian")                                                     },
        { "deu", QT_TR_NOOP("German")                                                       },
        { "ger", QT_TR_NOOP("German")                                                       },
        { "gmh", QT_TR_NOOP("German, Middle High (ca. 1050-1500)")                          },
        { "goh", QT_TR_NOOP("German, Old High (ca. 750-1050)")                              },
        { "gem", QT_TR_NOOP("Germanic (Other)")                                             },
        { "gil", QT_TR_NOOP("Gilbertese")                                                   },
        { "gon", QT_TR_NOOP("Gondi")                                                        },
        { "got", QT_TR_NOOP("Gothic")                                                       },
        { "grb", QT_TR_NOOP("Grebo")                                                        },
        { "grc", QT_TR_NOOP("Greek, Ancient (to 1453)")                                     },
        { "ell", QT_TR_NOOP("Greek, Modern (1453-)")                                        },
        { "gre", QT_TR_NOOP("Greek, Modern (1453-)")                                        },
        { "kal", QT_TR_NOOP("Greenlandic")                                                  },
        { "grn", QT_TR_NOOP("Guarani")                                                      },
        { "guj", QT_TR_NOOP("Gujarati")                                                     },
        { "hai", QT_TR_NOOP("Haida")                                                        },
        { "hau", QT_TR_NOOP("Hausa")                                                        },
        { "haw", QT_TR_NOOP("Hawaiian")                                                     },
        { "heb", QT_TR_NOOP("Hebrew")                                                       },
        { "her", QT_TR_NOOP("Herero")                                                       },
        { "hil", QT_TR_NOOP("Hiligaynon")                                                   },
        { "him", QT_TR_NOOP("Himachali")                                                    },
        { "hin", QT_TR_NOOP("Hindi")                                                        },
        { "hmo", QT_TR_NOOP("Hiri Motu")                                                    },
        { "hun", QT_TR_NOOP("Hungarian")                                                    },
        { "hup", QT_TR_NOOP("Hupa")                                                         },
        { "iba", QT_TR_NOOP("Iban")                                                         },
        { "ice", QT_TR_NOOP("Icelandic")                                                    },
        { "isl", QT_TR_NOOP("Icelandic")                                                    },
        { "ibo", QT_TR_NOOP("Igbo")                                                         },
        { "ijo", QT_TR_NOOP("Ijo")                                                          },
        { "ilo", QT_TR_NOOP("Iloko")                                                        },
        { "inc", QT_TR_NOOP("Indic (Other)")                                                },
        { "ine", QT_TR_NOOP("Indo-European (Other)")                                        },
        { "ind", QT_TR_NOOP("Indonesian")                                                   },
        { "ina", QT_TR_NOOP("Interlingua (International Auxiliary language Association)")   },
        { "ine", QT_TR_NOOP("Interlingue")                                                  },
        { "iku", QT_TR_NOOP("Inuktitut")                                                    },
        { "ipk", QT_TR_NOOP("Inupiak")                                                      },
        { "ira", QT_TR_NOOP("Iranian (Other)")                                              },
        { "gai", QT_TR_NOOP("Irish")                                                        },
        { "iri", QT_TR_NOOP("Irish")                                                        },
        { "sga", QT_TR_NOOP("Irish, Old (to 900)")                                          },
        { "mga", QT_TR_NOOP("Irish, Middle (900 - 1200)")                                   },
        { "iro", QT_TR_NOOP("Iroquoian languages")                                          },
        { "ita", QT_TR_NOOP("Italian")                                                      },
        { "jpn", QT_TR_NOOP("Japanese")                                                     },
        { "jav", QT_TR_NOOP("Javanese")                                                     },
        { "jaw", QT_TR_NOOP("Javanese")                                                     },
        { "jrb", QT_TR_NOOP("Judeo-Arabic")                                                 },
        { "jpr", QT_TR_NOOP("Judeo-Persian")                                                },
        { "kab", QT_TR_NOOP("Kabyle")                                                       },
        { "kac", QT_TR_NOOP("Kachin")                                                       },
        { "kam", QT_TR_NOOP("Kamba")                                                        },
        { "kan", QT_TR_NOOP("Kannada")                                                      },
        { "kau", QT_TR_NOOP("Kanuri")                                                       },
        { "kaa", QT_TR_NOOP("Kara-Kalpak")                                                  },
        { "kar", QT_TR_NOOP("Karen")                                                        },
        { "kas", QT_TR_NOOP("Kashmiri")                                                     },
        { "kaw", QT_TR_NOOP("Kawi")                                                         },
        { "kaz", QT_TR_NOOP("Kazakh")                                                       },
        { "kha", QT_TR_NOOP("Khasi")                                                        },
        { "khm", QT_TR_NOOP("Khmer")                                                        },
        { "khi", QT_TR_NOOP("Khoisan (Other)")                                              },
        { "kho", QT_TR_NOOP("Khotanese")                                                    },
        { "kik", QT_TR_NOOP("Kikuyu")                                                       },
        { "kin", QT_TR_NOOP("Kinyarwanda")                                                  },
        { "kir", QT_TR_NOOP("Kirghiz")                                                      },
        { "kom", QT_TR_NOOP("Komi")                                                         },
        { "kon", QT_TR_NOOP("Kongo")                                                        },
        { "kok", QT_TR_NOOP("Konkani")                                                      },
        { "kor", QT_TR_NOOP("Korean")                                                       },
        { "kpe", QT_TR_NOOP("Kpelle")                                                       },
        { "kro", QT_TR_NOOP("Kru")                                                          },
        { "kua", QT_TR_NOOP("Kuanyama")                                                     },
        { "kum", QT_TR_NOOP("Kumyk")                                                        },
        { "kur", QT_TR_NOOP("Kurdish")                                                      },
        { "kru", QT_TR_NOOP("Kurukh")                                                       },
        { "kus", QT_TR_NOOP("Kusaie")                                                       },
        { "kut", QT_TR_NOOP("Kutenai")                                                      },
        { "lad", QT_TR_NOOP("Ladino")                                                       },
        { "lah", QT_TR_NOOP("Lahnda")                                                       },
        { "lam", QT_TR_NOOP("Lamba")                                                        },
        { "oci", QT_TR_NOOP("Langue d'Oc (post 1500)")                                      },
        { "lao", QT_TR_NOOP("Lao")                                                          },
        { "lat", QT_TR_NOOP("Latin")                                                        },
        { "lav", QT_TR_NOOP("Latvian")                                                      },
        { "ltz", QT_TR_NOOP("Letzeburgesch")                                                },
        { "lez", QT_TR_NOOP("Lezghian")                                                     },
        { "lin", QT_TR_NOOP("Lingala")                                                      },
        { "lit", QT_TR_NOOP("Lithuanian")                                                   },
        { "loz", QT_TR_NOOP("Lozi")                                                         },
        { "lub", QT_TR_NOOP("Luba-Katanga")                                                 },
        { "lui", QT_TR_NOOP("Luiseno")                                                      },
        { "lun", QT_TR_NOOP("Lunda")                                                        },
        { "luo", QT_TR_NOOP("Luo (Kenya and Tanzania)")                                     },
        { "mac", QT_TR_NOOP("Macedonian")                                                   },
        { "mak", QT_TR_NOOP("Macedonian")                                                   },
        { "mad", QT_TR_NOOP("Madurese")                                                     },
        { "mag", QT_TR_NOOP("Magahi")                                                       },
        { "mai", QT_TR_NOOP("Maithili")                                                     },
        { "mak", QT_TR_NOOP("Makasar")                                                      },
        { "mlg", QT_TR_NOOP("Malagasy")                                                     },
        { "may", QT_TR_NOOP("Malay")                                                        },
        { "msa", QT_TR_NOOP("Malay")                                                        },
        { "mal", QT_TR_NOOP("Malayalam")                                                    },
        { "mlt", QT_TR_NOOP("Maltese")                                                      },
        { "man", QT_TR_NOOP("Mandingo")                                                     },
        { "mni", QT_TR_NOOP("Manipuri")                                                     },
        { "mno", QT_TR_NOOP("Manobo languages")                                             },
        { "max", QT_TR_NOOP("Manx")                                                         },
        { "mao", QT_TR_NOOP("Maori")                                                        },
        { "mri", QT_TR_NOOP("Maori")                                                        },
        { "mar", QT_TR_NOOP("Marathi")                                                      },
        { "chm", QT_TR_NOOP("Mari")                                                         },
        { "mah", QT_TR_NOOP("Marshall")                                                     },
        { "mwr", QT_TR_NOOP("Marwari")                                                      },
        { "mas", QT_TR_NOOP("Masai")                                                        },
        { "myn", QT_TR_NOOP("Mayan languages")                                              },
        { "men", QT_TR_NOOP("Mende")                                                        },
        { "mic", QT_TR_NOOP("Micmac")                                                       },
        { "min", QT_TR_NOOP("Minangkabau")                                                  },
        { "mis", QT_TR_NOOP("Miscellaneous (Other)")                                        },
        { "moh", QT_TR_NOOP("Mohawk")                                                       },
        { "mol", QT_TR_NOOP("Moldavian")                                                    },
        { "mkh", QT_TR_NOOP("Mon-Kmer (Other)")                                             },
        { "lol", QT_TR_NOOP("Mongo")                                                        },
        { "mon", QT_TR_NOOP("Mongolian")                                                    },
        { "mos", QT_TR_NOOP("Mossi")                                                        },
        { "mul", QT_TR_NOOP("Multiple languages")                                           },
        { "mun", QT_TR_NOOP("Munda languages")                                              },
        { "nau", QT_TR_NOOP("Nauru")                                                        },
        { "nav", QT_TR_NOOP("Navajo")                                                       },
        { "nde", QT_TR_NOOP("Ndebele, North")                                               },
        { "nbl", QT_TR_NOOP("Ndebele, South")                                               },
        { "ndo", QT_TR_NOOP("Ndongo")                                                       },
        { "nep", QT_TR_NOOP("Nepali")                                                       },
        { "new", QT_TR_NOOP("Newari")                                                       },
        { "nic", QT_TR_NOOP("Niger-Kordofanian (Other)")                                    },
        { "ssa", QT_TR_NOOP("Nilo-Saharan (Other)")                                         },
        { "niu", QT_TR_NOOP("Niuean")                                                       },
        { "non", QT_TR_NOOP("Norse, Old")                                                   },
        { "nai", QT_TR_NOOP("North American Indian (Other)")                                },
        { "nor", QT_TR_NOOP("Norwegian")                                                    },
        { "nno", QT_TR_NOOP("Norwegian (Nynorsk)")                                          },
        { "nub", QT_TR_NOOP("Nubian languages")                                             },
        { "nym", QT_TR_NOOP("Nyamwezi")                                                     },
        { "nya", QT_TR_NOOP("Nyanja")                                                       },
        { "nyn", QT_TR_NOOP("Nyankole")                                                     },
        { "nyo", QT_TR_NOOP("Nyoro")                                                        },
        { "nzi", QT_TR_NOOP("Nzima")                                                        },
        { "oji", QT_TR_NOOP("Ojibwa")                                                       },
        { "ori", QT_TR_NOOP("Oriya")                                                        },
        { "orm", QT_TR_NOOP("Oromo")                                                        },
        { "osa", QT_TR_NOOP("Osage")                                                        },
        { "oss", QT_TR_NOOP("Ossetic")                                                      },
        { "oto", QT_TR_NOOP("Otomian languages")                                            },
        { "pal", QT_TR_NOOP("Pahlavi")                                                      },
        { "pau", QT_TR_NOOP("Palauan")                                                      },
        { "pli", QT_TR_NOOP("Pali")                                                         },
        { "pam", QT_TR_NOOP("Pampanga")                                                     },
        { "pag", QT_TR_NOOP("Pangasinan")                                                   },
        { "pan", QT_TR_NOOP("Panjabi")                                                      },
        { "pap", QT_TR_NOOP("Papiamento")                                                   },
        { "paa", QT_TR_NOOP("Papuan-Australian (Other)")                                    },
        { "fas", QT_TR_NOOP("Persian")                                                      },
        { "per", QT_TR_NOOP("Persian")                                                      },
        { "peo", QT_TR_NOOP("Persian, Old (ca 600 - 400 B.C.)")                             },
        { "phn", QT_TR_NOOP("Phoenician")                                                   },
        { "pol", QT_TR_NOOP("Polish")                                                       },
        { "pon", QT_TR_NOOP("Ponape")                                                       },
        { "por", QT_TR_NOOP("Portuguese")                                                   },
        { "pra", QT_TR_NOOP("Prakrit languages")                                            },
        { "pro", QT_TR_NOOP("Provencal, Old (to 1500)")                                     },
        { "pus", QT_TR_NOOP("Pushto")                                                       },
        { "que", QT_TR_NOOP("Quechua")                                                      },
        { "roh", QT_TR_NOOP("Rhaeto-Romance")                                               },
        { "raj", QT_TR_NOOP("Rajasthani")                                                   },
        { "rar", QT_TR_NOOP("Rarotongan")                                                   },
        { "roa", QT_TR_NOOP("Romance (Other)")                                              },
        { "ron", QT_TR_NOOP("Romanian")                                                     },
        { "rum", QT_TR_NOOP("Romanian")                                                     },
        { "rom", QT_TR_NOOP("Romany")                                                       },
        { "run", QT_TR_NOOP("Rundi")                                                        },
        { "rus", QT_TR_NOOP("Russian")                                                      },
        { "sal", QT_TR_NOOP("Salishan languages")                                           },
        { "sam", QT_TR_NOOP("Samaritan Aramaic")                                            },
        { "smi", QT_TR_NOOP("Sami languages")                                               },
        { "smo", QT_TR_NOOP("Samoan")                                                       },
        { "sad", QT_TR_NOOP("Sandawe")                                                      },
        { "sag", QT_TR_NOOP("Sango")                                                        },
        { "san", QT_TR_NOOP("Sanskrit")                                                     },
        { "srd", QT_TR_NOOP("Sardinian")                                                    },
        { "sco", QT_TR_NOOP("Scots")                                                        },
        { "sel", QT_TR_NOOP("Selkup")                                                       },
        { "sem", QT_TR_NOOP("Semitic (Other)")                                              },
        { "scr", QT_TR_NOOP("Serbo-Croatian")                                               },
        { "srr", QT_TR_NOOP("Serer")                                                        },
        { "shn", QT_TR_NOOP("Shan")                                                         },
        { "sna", QT_TR_NOOP("Shona")                                                        },
        { "sid", QT_TR_NOOP("Sidamo")                                                       },
        { "bla", QT_TR_NOOP("Siksika")                                                      },
        { "snd", QT_TR_NOOP("Sindhi")                                                       },
        { "sin", QT_TR_NOOP("Singhalese")                                                   },
        { "sit", QT_TR_NOOP("Sino-Tibetan (Other)")                                         },
        { "sio", QT_TR_NOOP("Siouan languages")                                             },
        { "sla", QT_TR_NOOP("Slavic (Other)")                                               },
        { "ssw", QT_TR_NOOP("Siswant")                                                      },
        { "slk", QT_TR_NOOP("Slovak")                                                       },
        { "slo", QT_TR_NOOP("Slovak")                                                       },
        { "slv", QT_TR_NOOP("Slovenian")                                                    },
        { "sog", QT_TR_NOOP("Sogdian")                                                      },
        { "som", QT_TR_NOOP("Somali")                                                       },
        { "son", QT_TR_NOOP("Songhai")                                                      },
        { "wen", QT_TR_NOOP("Sorbian languages")                                            },
        { "nso", QT_TR_NOOP("Sotho, Northern")                                              },
        { "sot", QT_TR_NOOP("Sotho, Southern")                                              },
        { "sai", QT_TR_NOOP("South American Indian (Other)")                                },
        { "esl", QT_TR_NOOP("Spanish")                                                      },
        { "spa", QT_TR_NOOP("Spanish")                                                      },
        { "suk", QT_TR_NOOP("Sukuma")                                                       },
        { "sux", QT_TR_NOOP("Sumerian")                                                     },
        { "sun", QT_TR_NOOP("Sudanese")                                                     },
        { "sus", QT_TR_NOOP("Susu")                                                         },
        { "swa", QT_TR_NOOP("Swahili")                                                      },
        { "ssw", QT_TR_NOOP("Swazi")                                                        },
        { "sve", QT_TR_NOOP("Swedish")                                                      },
        { "swe", QT_TR_NOOP("Swedish")                                                      },
        { "syr", QT_TR_NOOP("Syriac")                                                       },
        { "tgl", QT_TR_NOOP("Tagalog")                                                      },
        { "tah", QT_TR_NOOP("Tahitian")                                                     },
        { "tgk", QT_TR_NOOP("Tajik")                                                        },
        { "tmh", QT_TR_NOOP("Tamashek")                                                     },
        { "tam", QT_TR_NOOP("Tamil")                                                        },
        { "tat", QT_TR_NOOP("Tatar")                                                        },
        { "tel", QT_TR_NOOP("Telugu")                                                       },
        { "ter", QT_TR_NOOP("Tereno")                                                       },
        { "tha", QT_TR_NOOP("Thai")                                                         },
        { "bod", QT_TR_NOOP("Tibetan")                                                      },
        { "tib", QT_TR_NOOP("Tibetan")                                                      },
        { "tig", QT_TR_NOOP("Tigre")                                                        },
        { "tir", QT_TR_NOOP("Tigrinya")                                                     },
        { "tem", QT_TR_NOOP("Timne")                                                        },
        { "tiv", QT_TR_NOOP("Tivi")                                                         },
        { "tli", QT_TR_NOOP("Tlingit")                                                      },
        { "tog", QT_TR_NOOP("Tonga (Nyasa)")                                                },
        { "ton", QT_TR_NOOP("Tonga (Tonga Islands)")                                        },
        { "tru", QT_TR_NOOP("Truk")                                                         },
        { "tsi", QT_TR_NOOP("Tsimshian")                                                    },
        { "tso", QT_TR_NOOP("Tsonga")                                                       },
        { "tsn", QT_TR_NOOP("Tswana")                                                       },
        { "tum", QT_TR_NOOP("Tumbuka")                                                      },
        { "tur", QT_TR_NOOP("Turkish")                                                      },
        { "ota", QT_TR_NOOP("Turkish, Ottoman (1500 - 1928)")                               },
        { "tuk", QT_TR_NOOP("Turkmen")                                                      },
        { "tyv", QT_TR_NOOP("Tuvinian")                                                     },
        { "twi", QT_TR_NOOP("Twi")                                                          },
        { "uga", QT_TR_NOOP("Ugaritic")                                                     },
        { "uig", QT_TR_NOOP("Uighur")                                                       },
        { "ukr", QT_TR_NOOP("Ukrainian")                                                    },
        { "umb", QT_TR_NOOP("Umbundu")                                                      },
        { "und", QT_TR_NOOP("Undetermined")                                                 },
        { "urd", QT_TR_NOOP("Urdu")                                                         },
        { "uzb", QT_TR_NOOP("Uzbek")                                                        },
        { "vai", QT_TR_NOOP("Vai")                                                          },
        { "ven", QT_TR_NOOP("Venda")                                                        },
        { "vie", QT_TR_NOOP("Vietnamese")                                                   },
        { "vol", QT_TR_NOOP("Volapük")                                                      },
        { "vot", QT_TR_NOOP("Votic")                                                        },
        { "wak", QT_TR_NOOP("Wakashan languages")                                           },
        { "wal", QT_TR_NOOP("Walamo")                                                       },
        { "war", QT_TR_NOOP("Waray")                                                        },
        { "was", QT_TR_NOOP("Washo")                                                        },
        { "cym", QT_TR_NOOP("Welsh")                                                        },
        { "wel", QT_TR_NOOP("Welsh")                                                        },
        { "wol", QT_TR_NOOP("Wolof")                                                        },
        { "xho", QT_TR_NOOP("Xhosa")                                                        },
        { "sah", QT_TR_NOOP("Yakut")                                                        },
        { "yao", QT_TR_NOOP("Yao")                                                          },
        { "yap", QT_TR_NOOP("Yap")                                                          },
        { "yid", QT_TR_NOOP("Yiddish")                                                      },
        { "yor", QT_TR_NOOP("Yoruba")                                                       },
        { "zap", QT_TR_NOOP("Zapotec")                                                      },
        { "zen", QT_TR_NOOP("Zenaga")                                                       },
        { "zha", QT_TR_NOOP("Zhuang")                                                       },
        { "zul", QT_TR_NOOP("Zulu")                                                         },
        { "zun", QT_TR_NOOP("Zuni")                                                         },
        { NULL, NULL } };

  return codes;
}

} // End of namespaces
