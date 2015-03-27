using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using winbind_wrapper.armor_up_wrapper;

namespace winbind_wrapper
{
    class Program
    {
        static void Main(string[] args)
        {
            // Wrapper.Initialize should be called only once for the whole lifetime 
            // of your program. Do not call it every time when you do a search :)
            Wrapper.Initialize("d:/pf/projects/monster-avengers/dataset/MH4GU");

            // Wrapper.Search is the entry point of the wrapper. It returns a list of 
            // ArmorSet objects as result. Please See definition of ArmorSet for details.
            List<ArmorSet> result = Wrapper.Search("(:weapon-holes 2) (:skill 25 15)");

            // Iterate over the answers, and print them.
            foreach (ArmorSet answer in result) {
                answer.DebugPrint();
                Console.WriteLine("--------------------");
            }
        }
    }
}
