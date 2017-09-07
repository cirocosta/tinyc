#include "./names.h"

static const char* tc_names_left[] =
  {
    "admiring",   "adoring",  "affectionate",  "agitated",   "amazing",
    "angry",      "awesome",  "blissful",      "boring",     "brave",
    "clever",     "cocky",    "compassionate", "competent",  "condescending",
    "confident",  "cranky",   "dazzling",      "determined", "distracted",
    "dreamy",     "eager",    "ecstatic",      "elastic",    "elated",
    "elegant",    "eloquent", "epic",          "fervent",    "festive",
    "flamboyant", "focused",  "friendly",      "frosty",     "gallant",
    "gifted",     "goofy",    "gracious",      "happy",      "hardcore",
    "heuristic",  "hopeful",  "hungry",        "infallible", "inspiring",
    "jolly",      "jovial",   "keen",          "kind",       "laughing",
    "loving",     "lucid",    "mystifying",    "modest",     "musing",
    "naughty",    "nervous",  "nifty",         "nostalgic",  "objective",
    "optimistic", "peaceful", "pedantic",      "pensive",    "practical",
    "priceless",  "quirky",   "quizzical",     "relaxed",    "reverent",
    "romantic",   "sad",      "serene",        "sharp",      "silly",
    "sleepy",     "stoic",    "stupefied",     "suspicious", "tender",
    "thirsty",    "trusting", "unruffled",     "upbeat",     "vibrant",
    "vigilant",   "vigorous", "wizardly",      "wonderful",  "xenodochial",
    "youthful",   "zealous",  "zen",
  };

static const char* tc_names_right[] =
  { "albattani",   "allen",      "almeida",      "agnesi",        "archimedes",
    "ardinghelli", "aryabhata",  "austin",       "babbage",       "banach",
    "bardeen",     "bartik",     "bassi",        "beaver",        "bell",
    "benz",        "bhabha",     "bhaskara",     "blackwell",     "bohr",
    "booth",       "borg",       "bose",         "boyd",          "brahmagupta",
    "brattain",    "brown",      "carson",       "chandrasekhar", "shannon",
    "clarke",      "colden",     "cori",         "cray",          "curran",
    "curie",       "darwin",     "davinci",      "dijkstra",      "dubinsky",
    "easley",      "edison",     "einstein",     "elion",         "engelbart",
    "euclid",      "euler",      "fermat",       "fermi",         "feynman",
    "franklin",    "galileo",    "gates",        "goldberg",      "goldstine",
    "goldwasser",  "golick",     "goodall",      "haibt",         "hamilton",
    "hawking",     "heisenberg", "hermann",      "heyrovsky",     "hodgkin",
    "hoover",      "hopper",     "hugle",        "hypatia",       "jackson",
    "jang",        "jennings",   "jepsen",       "johnson",       "keller",
    "kepler",      "khorana",    "kilby",        "kirch",         "knuth",
    "kowalevski",  "lalande",    "lamarr",       "lamport",       "leakey",
    "leavitt",     "lewin",      "lichterman",   "liskov",        "lovelace",
    "mayer",       "mccarthy",   "mcclintock",   "mclean",        "mcnulty",
    "meitner",     "meninsky",   "mestorf",      "minsky",        "mirzakhani",
    "morse",       "murdock",    "neumann",      "newton",        "nightingale",
    "nobel",       "noether",    "northcutt",    "noyce",         "panini",
    "pare",        "pasteur",    "payne",        "perlman",       "pike",
    "poincare",    "poitras",    "ptolemy",      "raman",         "ramanujan",
    "ride",        "montalcini", "ritchie",      "roentgen",      "rosalind",
    "saha",        "sammet",     "shaw",         "shirley",       "shockley",
    "sinoussi",    "snyder",     "spence",       "stallman",      "stonebraker",
    "swanson",     "swartz",     "swirles",      "tesla",         "thompson",
    "torvalds",    "turing",     "varahamihira", "visvesvaraya",  "volhard",
    "wescoff",     "wiles",      "williams",     "wilson",        "wing",
    "wozniak",     "wright",     "yalow",        "yonath" };

static const int tc_len_names_left =
  sizeof(tc_names_left) / sizeof(tc_names_left[0]);
static const int tc_len_names_right =
  sizeof(tc_names_right) / sizeof(tc_names_right[0]);

int
tc_names_fill(char* buff, size_t len)
{
	int left_ndx = rand() % tc_len_names_left;
	int right_ndx = rand() % tc_len_names_right;

	return snprintf(buff, len, "%s_%s", tc_names_left[left_ndx],
	                tc_names_right[right_ndx]);
}
