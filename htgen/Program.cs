using System;
using System.IO;
using System.Web;

namespace HTGen
{
    class Generator
    {
        private string inFilePath;
        public Generator(string inPath)
        {
            inFilePath = inPath;
        }
        public void Run()
        {
            FileInfo inFileInfo = new FileInfo(inFilePath);
            string[] lines = File.ReadAllLines(inFileInfo.FullName, System.Text.Encoding.UTF8);
            bool inCode = false;
            string code = "";
            foreach (string line in lines)
            {
                string result = null;
                if (!inCode)
                {
                    if (line.StartsWith('!'))
                        result = MakeHeading(line);
                    else if (line.StartsWith("<<<"))
                    {
                        code = "";
                        inCode = true;
                    }
                    else
                        result = MakeParagraph(line);
                }
                else
                {
                    if (line.StartsWith(">>>"))
                    {
                        inCode = false;
                        result = String.Format("<pre>{0}</pre>", code);
                    }
                    else
                        code += "\n" + HttpUtility.HtmlEncode(line);
                }
                if (!inCode)
                    Console.WriteLine(result);
            }
        }
        private string MakeHeading(string source)
        {
            int level = 0;
            foreach (char ch in source)
            {
                if (ch == '!')
                {
                    level += 1;
                    if (level == 5)
                        break;
                }
                else
                    break;
            }
            string content = HttpUtility.HtmlEncode(source.Substring(level));
            string html = String.Format("<h{0}>{1}</h{0}>", level, content);
            return html;
        }
        private string MakeParagraph(string source)
        {
            string content = HttpUtility.HtmlEncode(source);
            string html = String.Format("<p>{0}</p>", content);
            return html;
        }
    }
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length > 0)
            {
                Generator gen = new Generator(args[0]);
                try
                {
                    gen.Run();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                }
            }
            Console.WriteLine("Done");
        }
    }
}
