#include "advDFG.hh"

#include <iostream>
#include <unordered_map>

namespace instr {

static const char *nodeTypeToString(NodeType type) {
	switch (type) {
		case NodeType::EntryLabel:
			return "EntryLabel";
		case NodeType::Statement:
			return "Statement";
		case NodeType::ExitStatement:
			return "ExitStatement";
	}
	return "Unknown";
}

void advDFG::printToStdout() const {
	std::unordered_map<const advDFGNode*, int> nodeIds;
	int nextId = 0;
	for (auto *node : nodes) {
		nodeIds[node] = nextId++;
	}

	std::cout << "    node_count=" << nodes.size() << std::endl;
	for (auto *node : nodes) {
		std::cout << "    node#" << nodeIds[node]
				  << " type=" << nodeTypeToString(node->type);

		if (node->quadStatement != nullptr) {
			std::cout << " quad_kind=" << quad::quadKindToString(node->quadStatement->kind);
		}

		std::cout << " def_temp=" << node->tempDefined
				  << " def_chain=" << node->chainDefined
				  << " uses_temps={";

		bool first = true;
		for (int t : node->tempsUsed) {
			if (!first) {
				std::cout << ",";
			}
			std::cout << t;
			first = false;
		}

		std::cout << "} uses_chain=" << node->chainUsed;

		std::cout << " preds={";
		first = true;
		for (auto *pred : node->predecessors) {
			if (!first) {
				std::cout << ",";
			}
			std::cout << nodeIds[pred];
			first = false;
		}

		std::cout << "} succs={";
		first = true;
		for (auto *succ : node->successors) {
			if (!first) {
				std::cout << ",";
			}
			std::cout << nodeIds[succ];
			first = false;
		}

		std::cout << "}" << std::endl;
	}
}

void advDFGprog::printAllGraphsToStdout() const {
	std::cout << "advDFG program dump" << std::endl;
	std::cout << "  function_count=" << fungraph.size() << std::endl;

	int funcIndex = 0;
	for (auto *funcRecord : fungraph) {
		std::cout << "  func[" << funcIndex++ << "]";
		if (funcRecord != nullptr && funcRecord->quadFunc != nullptr) {
			std::cout << " name=" << funcRecord->quadFunc->funcname;
		}
		std::cout << std::endl;

		if (funcRecord == nullptr) {
			continue;
		}

		std::cout << "    block_count=" << funcRecord->blockgraph.size() << std::endl;
		int blockIndex = 0;
		for (auto *blockRecord : funcRecord->blockgraph) {
			std::cout << "    block[" << blockIndex++ << "]";
			if (blockRecord != nullptr && blockRecord->quadBlock != nullptr &&
				blockRecord->quadBlock->entry_label != nullptr) {
				std::cout << " entry=L" << blockRecord->quadBlock->entry_label->num;
			}
			std::cout << std::endl;

			if (blockRecord == nullptr) {
				continue;
			}

			blockRecord->graph.printToStdout();
		}
	}
}

} // namespace instr