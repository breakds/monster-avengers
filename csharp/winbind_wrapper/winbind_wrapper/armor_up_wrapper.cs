using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using winbind_wrapper.parser;
using System.Runtime.InteropServices;

namespace winbind_wrapper {
	namespace armor_up_wrapper {
		enum Instruction {
			GEAR = 0,
			HEAD,
			BODY,
			ARMS,
			WAIST,
			LEGS,
			AMULET,
			COMPLETE,
			INVALID,
		}

		struct ArmorSetPiece {
			public int id;
			public List < int > jewel_ids;

			public static bool Parse(
			ref Tokenizer tokenizer,
			out ArmorSetPiece piece) {
				piece = new ArmorSetPiece();
				piece.id = 0;
				tokenizer.Expect(TokenName.OPEN_PARENTHESIS);
				if (!tokenizer.ExpectNumber(out piece.id)) {
					return false;
				}
				if (!tokenizer.ExpectIntegerList(out piece.jewel_ids)) {
					return false;
				}

				return tokenizer.Expect(TokenName.CLOSE_PARENTHESIS);
			}

			public void DebugPrint() {
				Console.Write("id = {0}, jewels = ", id);
				foreach(int jewel_id in jewel_ids) {
					Console.Write("{0} ", jewel_id);
				}

				Console.WriteLine("");
			}
		}

		struct Effect {
			public int skill_id;
			public int points;
		}

		struct Amulet {
			public List < Effect > effects;
			public List < int > jewel_ids;

			public static bool Parse(ref Tokenizer tokenizer, out Amulet result) {
				tokenizer.Expect(TokenName.OPEN_PARENTHESIS);
				List < int > temp;
				result = new Amulet();
				result.effects = new List < Effect > ();
				result.jewel_ids = new List < int > ();
				if (tokenizer.ExpectIntegerList(out temp)) {
					if (temp.Count >= 2) {
						Effect effect = new Effect();
						effect.skill_id = temp[0];
						effect.points = temp[1];
						result.effects.Add(effect);
					}
					if (temp.Count >= 4) {
						Effect effect = new Effect();
						effect.skill_id = temp[2];
						effect.points = temp[3];
						result.effects.Add(effect);
					}
				} else {
					return false;
				}

				if (!tokenizer.ExpectIntegerList(out result.jewel_ids)) {
					return false;
				}
				return tokenizer.Expect(TokenName.CLOSE_PARENTHESIS);
			}

			public void DebugPrint() {
				foreach(Effect effect in effects) {
					Console.Write(" Effect({0} {1}) ",
					effect.skill_id,
					effect.points);
				}
				Console.Write(" jewels = ");
				foreach(int jewel_id in jewel_ids) {
					Console.Write("{0} ", jewel_id);
				}
				Console.WriteLine("");
			}
		}

		struct ArmorSet {
			public ArmorSetPiece gear;
			public ArmorSetPiece head;
			public ArmorSetPiece body;
			public ArmorSetPiece arms;
			public ArmorSetPiece waist;
			public ArmorSetPiece legs;
			public Amulet amulet;
			int weapon_slots;

			public static bool Parse(ref Tokenizer tokenizer, out ArmorSet answer) {
				answer = new ArmorSet();
				if (!tokenizer.Expect(TokenName.OPEN_PARENTHESIS)) {
					return false;
				}

				do {
					Instruction instruction = ReadInstruction(ref tokenizer);
					switch (instruction) {
						case Instruction.GEAR:
							if (!ArmorSetPiece.Parse(ref tokenizer, out answer.gear)) {
								return false;
							}
							answer.weapon_slots = answer.gear.id;
							break;
						case Instruction.HEAD:
							if (!ArmorSetPiece.Parse(ref tokenizer, out answer.head)) {
								return false;
							}
							break;
						case Instruction.BODY:
							if (!ArmorSetPiece.Parse(ref tokenizer, out answer.body)) {
								return false;
							}
							break;
						case Instruction.ARMS:
							if (!ArmorSetPiece.Parse(ref tokenizer, out answer.arms)) {
								return false;
							}
							break;
						case Instruction.WAIST:
							if (!ArmorSetPiece.Parse(ref tokenizer, out answer.waist)) {
								return false;
							}
							break;
						case Instruction.LEGS:
							if (!ArmorSetPiece.Parse(ref tokenizer, out answer.legs)) {
								return false;
							}
							break;
						case Instruction.AMULET:
							if (!Amulet.Parse(ref tokenizer, out answer.amulet)) {
								return false;
							}
							break;
						case Instruction.COMPLETE:
							return true;
						default:
							return false;
					}
				} while (!tokenizer.Empty());

				return false;
			}

			public static Instruction ReadInstruction(ref Tokenizer tokenizer) {
				Token token = new Token();
				tokenizer.Next(out token);
				if (token.name != TokenName.KEYWORD) {
					if (token.name == TokenName.CLOSE_PARENTHESIS) {
						return Instruction.COMPLETE;
					}
					return Instruction.INVALID;
				}
				if (token.value.ToLower() == "gear") {
					return Instruction.GEAR;
				} else if (token.value.ToLower() == "head") {
					return Instruction.HEAD;
				} else if (token.value.ToLower() == "body") {
					return Instruction.BODY;
				} else if (token.value.ToLower() == "arms") {
					return Instruction.ARMS;
				} else if (token.value.ToLower() == "waist") {
					return Instruction.WAIST;
				} else if (token.value.ToLower() == "legs") {
					return Instruction.LEGS;
				} else if (token.value.ToLower() == "amulet") {
					return Instruction.AMULET;
				} else {
					return Instruction.INVALID;
				}
			}

			public void DebugPrint() {
				Console.Write("gear: ");
				gear.DebugPrint();
				Console.Write("head: ");
				head.DebugPrint();
				Console.Write("body: ");
				body.DebugPrint();
				Console.Write("arms: ");
				arms.DebugPrint();
				Console.Write("waist: ");
				waist.DebugPrint();
				Console.Write("legs: ");
				legs.DebugPrint();
				Console.Write("amulet: ");
				amulet.DebugPrint();
			}
		}

	    class Wrapper {
            // You should be able to use the dll if it is located at the same directory of the binary.
            [DllImport("winbind.dll",
                        CharSet = CharSet.Unicode)]
            private static extern IntPtr DoSearch(string text);

            [DllImport("winbind.dll")]
            public static extern void Initialize(string dataset);

            public static List<ArmorSet> Search(string query) {
                List<ArmorSet> result = new List<ArmorSet>();
                Tokenizer tokenizer = new Tokenizer(
                    Marshal.PtrToStringAnsi(DoSearch(query)));
                ArmorSet answer = new ArmorSet();
                while (ArmorSet.Parse(ref tokenizer, out answer)) {
                    result.Add(answer);
                }
                return result;
            }
	    }
    }
}