#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
using namespace std;

// JSON parser for our specific format
class SimpleJSONParser {
private:
    string removeSpaces(const string& str) {
        string result = str;
        result.erase(remove(result.begin(), result.end(), ' '), result.end());
        result.erase(remove(result.begin(), result.end(), '\n'), result.end());
        result.erase(remove(result.begin(), result.end(), '\t'), result.end());
        return result;
    }

    string extractValue(const string& json, const string& key) {
        string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == string::npos) return "";
        
        pos += searchKey.length();
        
        // Skip whitespace and quotes
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) pos++;
        
        if (pos < json.length() && json[pos] == '"') {
            pos++; // Skip opening quote
            size_t end = json.find('"', pos);
            if (end != string::npos) {
                return json.substr(pos, end - pos);
            }
        } else {
            // Number without quotes
            size_t end = pos;
            while (end < json.length() && (isdigit(json[end]) || json[end] == '.')) end++;
            if (json[end] == ',' || json[end] == '}') {
                return json.substr(pos, end - pos);
            }
        }
        return "";
    }

public:
    struct Point {
        int x;
        long long y;
    };

    struct TestCase {
        int n, k;
        vector<Point> points;
    };

    TestCase parseJSON(const string& filename) {
        TestCase testCase;
        ifstream file(filename);
        string json, line;
        
        while (getline(file, line)) {
            json += line;
        }
        file.close();
        
        json = removeSpaces(json);
        
        // Extract n and k
        testCase.n = stoi(extractValue(json, "n"));
        testCase.k = stoi(extractValue(json, "k"));
        
        cout << "n = " << testCase.n << ", k = " << testCase.k << endl;
        
        // Extract points
        for (int i = 1; i <= testCase.n; i++) {
            string iStr = to_string(i);
            
            // Find the section for this point
            string searchPattern = "\"" + iStr + "\":{";
            size_t pos = json.find(searchPattern);
            if (pos == string::npos) continue;
            
            size_t end = json.find("}", pos);
            if (end == string::npos) continue;
            
            string pointSection = json.substr(pos, end - pos + 1);
            
            string baseStr = extractValue(pointSection, "base");
            string valueStr = extractValue(pointSection, "value");
            
            if (!baseStr.empty() && !valueStr.empty()) {
                int base = stoi(baseStr);
                long long y = convertToDecimal(valueStr, base);
                
                testCase.points.push_back({i, y});
                cout << "Point " << i << ": base=" << base << ", value=" << valueStr 
                     << " -> decimal=" << y << endl;
            }
        }
        
        return testCase;
    }

private:
    long long convertToDecimal(const string& value, int base) {
        long long result = 0;
        long long power = 1;
        
        for (int i = value.length() - 1; i >= 0; i--) {
            char c = value[i];
            int digit;
            
            if (c >= '0' && c <= '9') {
                digit = c - '0';
            } else if (c >= 'a' && c <= 'z') {
                digit = c - 'a' + 10;
            } else if (c >= 'A' && c <= 'Z') {
                digit = c - 'A' + 10;
            } else {
                continue; // Skip invalid characters
            }
            
            if (digit >= base) {
                cout << "Warning: Invalid digit " << c << " for base " << base << endl;
                continue;
            }
            
            result += digit * power;
            power *= base;
        }
        
        return result;
    }
};

class SecretSolver {
public:
    // Lagrange interpolation to find f(0)
    static long long findSecret(const vector<SimpleJSONParser::Point>& points, int k) {
        cout << "\nUsing first " << k << " points for Lagrange interpolation:" << endl;
        
        // Use only first k points
        vector<SimpleJSONParser::Point> selectedPoints(points.begin(), points.begin() + k);
        
        long long secret = 0;
        
        for (int i = 0; i < k; i++) {
            cout << "Processing point (" << selectedPoints[i].x << ", " << selectedPoints[i].y << ")" << endl;
            
            // Calculate Lagrange basis polynomial L_i(0)
            long long numerator = 1;
            long long denominator = 1;
            
            for (int j = 0; j < k; j++) {
                if (i != j) {
                    numerator *= (0 - selectedPoints[j].x);  // We want f(0)
                    denominator *= (selectedPoints[i].x - selectedPoints[j].x);
                }
            }
            
            cout << "  Lagrange coefficient: " << numerator << "/" << denominator << endl;
            
            // Add this term to the result
            secret += selectedPoints[i].y * numerator / denominator;
        }
        
        return secret;
    }
};

int main() {
    cout << "=== Shamir's Secret Sharing Solver ===" << endl;
    
    // Test Case 1
    cout << "\n--- Test Case 1 ---" << endl;
    SimpleJSONParser parser;
    
    // Create test case 1 file
    ofstream testFile1("testcase1.json");
    testFile1 << R"({
    "keys": {
        "n": 4,
        "k": 3
    },
    "1": {
        "base": "10",
        "value": "4"
    },
    "2": {
        "base": "2",
        "value": "111"
    },
    "3": {
        "base": "10",
        "value": "12"
    },
    "6": {
        "base": "4",
        "value": "213"
    }
})";
    testFile1.close();
    
    auto testCase1 = parser.parseJSON("testcase1.json");
    long long secret1 = SecretSolver::findSecret(testCase1.points, testCase1.k);
    cout << "\nSecret for Test Case 1: " << secret1 << endl;
    
    // Test Case 2
    cout << "\n--- Test Case 2 ---" << endl;
    
    // Create test case 2 file
    ofstream testFile2("testcase2.json");
    testFile2 << R"({
"keys": {
    "n": 10,
    "k": 7
  },
  "1": {
    "base": "6",
    "value": "13444211440455345511"
  },
  "2": {
    "base": "15",
    "value": "aed7015a346d63"
  },
  "3": {
    "base": "15",
    "value": "6aeeb69631c227c"
  },
  "4": {
    "base": "16",
    "value": "e1b5e05623d881f"
  },
  "5": {
    "base": "8",
    "value": "316034514573652620673"
  },
  "6": {
    "base": "3",
    "value": "2122212201122002221120200210011020220200"
  },
  "7": {
    "base": "3",
    "value": "20120221122211000100210021102001201112121"
  },
  "8": {
    "base": "6",
    "value": "20220554335330240002224253"
  },
  "9": {
    "base": "12",
    "value": "45153788322a1255483"
  },
  "10": {
    "base": "7",
    "value": "1101613130313526312514143"
  }
})";
    testFile2.close();
    
    auto testCase2 = parser.parseJSON("testcase2.json");
    long long secret2 = SecretSolver::findSecret(testCase2.points, testCase2.k);
    cout << "\nSecret for Test Case 2: " << secret2 << endl;
    
    cout << "\n=== Summary ===" << endl;
    cout << "Test Case 1 Secret: " << secret1 << endl;
    cout << "Test Case 2 Secret: " << secret2 << endl;
    
    return 0;
}
