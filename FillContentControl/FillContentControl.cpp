#include "duckx.hpp"
#include <pugixml.hpp>
#include <vector>
#include <iostream>
#include <argparse/argparse.hpp>

struct findContentControlTag
{
    bool operator()(pugi::xml_attribute attr) const
    {
        return strcmp(attr.name(), "w:val") == 0;
    }

    bool operator()(pugi::xml_node node) const
    {
        return strcmp(node.name(), "w:tag") == 0;
    }
};

struct findContentControlText
{
    bool operator()(pugi::xml_attribute attr) const
    {
        return strcmp(attr.name(), "w:t") == 0;
    }

    bool operator()(pugi::xml_node node) const
    {
        return strcmp(node.name(), "w:t") == 0;
    }
};

struct contentControlFinder : pugi::xml_tree_walker
{
    std::vector <pugi::xml_node> nodes;

    virtual bool for_each(pugi::xml_node& node)
    {
        if (strcmp(node.name(), "w:sdt") == 0) {
            nodes.push_back(node);
        }

        return true; // continue traversal
    }
};

void fillContentControl(std::vector <pugi::xml_node> nodes, std::string tag, std::string text){
    for (auto& node : nodes) {
        auto tagNode = node.find_node(findContentControlTag());
        if (strcmp(tagNode.attribute("w:val").value(), tag.c_str()) == 0) {
            auto textNode = node.find_node(findContentControlText());
            textNode.text().set(text.c_str());
        }
    }
}

int main(int argc, char* argv[])
{
    argparse::ArgumentParser program("Fill DOCX content control");

    program.add_argument("-f", "--file")
        .required()
        .help("specify the input file.");

    program.add_argument("-a", "--tag")
        .required()
        .help("tag.");

    program.add_argument("-t", "--text")
        .required()
        .help("text.");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        exit(0);
    }

    std::string filepath = program.get<std::string>("-f");
    std::string tag = program.get<std::string>("-a");
    std::string text = program.get<std::string>("-t");


    // open doc
    duckx::Document doc(filepath);
    doc.open();
    // find all nodes with Content Control
    contentControlFinder cc;
    doc.document.traverse(cc);
    // fill CC
    fillContentControl(cc.nodes, tag, text);
    // save doc
    doc.save();
}
