#include "Properties.h"

/***********************
 *     Properties     *
 ***********************/

/* Static */
int Properties::ARRAY_SIZE = 128;
int Properties::PROPERTY_MAX_SIZE = 512;

/* Construct */
Properties::Properties() : Properties(nullptr) {}

Properties::Properties(const char *path) {
    properties = new Node *[ARRAY_SIZE];

    for (int i = 0; i < ARRAY_SIZE; i++) {
        properties[i] = nullptr;
    }

    initSuccess = true;
    propertySize = 0;
    if (path != nullptr && !load(path)) {
        initSuccess = false;
    }
}

Properties::~Properties() {
    clear();
    delete[] properties;
}

/* Private */
size_t Properties::fileSize(FILE *file) {
    long locationCache;
    size_t size;

    locationCache = ftell(file);
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, locationCache, SEEK_SET);
    return size;
}

char *Properties::readFile(FILE *file) {
    // get file size
    size_t bufferSize = fileSize(file);

    if (bufferSize == 0) {
        // puts("Properties::readFile: file is empty.");
        return nullptr;
    }

    // set cursor to file header
    fseek(file, (long) 0, SEEK_SET);

    // create buffer
    char *fileBuffer = new char[bufferSize + 1];
    fread(fileBuffer, sizeof(char), bufferSize, file);

    // end of buffer
    fileBuffer[bufferSize] = '\0';

    return fileBuffer;
}

bool Properties::isBlank(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

void Properties::analyze(const char *str) {
    enum status {
        ready,
        key,
        division,
        value,
        annotation,
        error
    };

    status state = ready;
    bool isCompleted = false;
    unsigned long count = 0;
    int propertyCount = 0;
    char *property = new char[PROPERTY_MAX_SIZE + 1];
    Node node;

    // start analyze
    while (!isCompleted) {
        switch (state) {
            case ready:
                if (!isBlank(str[count])) {
                    if (str[count] == '#') {
                        state = annotation;
                    } else if (str[count] == '\0') {
                        isCompleted = true;
                    } else {
                        state = key;
                        propertyCount = 0;
                        property[propertyCount++] = str[count];
                    }
                }
                break;

            case key:
                if (str[count] != '=') {
                    if (str[count] == '\0' || str[count] == '\n') {
                        state = error;
                    } else {
                        property[propertyCount++] = str[count];
                    }
                } else {
                    state = division;

                    // end of key
                    property[propertyCount] = '\0';

                    // remove blank
                    while (isBlank(property[--propertyCount])) {
                        property[propertyCount] = '\0';
                    }
                    node.setKey(property);
                }
                break;

            case division:
                if (!isBlank(str[count])) {
                    state = value;
                    propertyCount = 0;
                    property[propertyCount++] = str[count];
                }

                break;

            case value:
                if (str[count] != '\n' && str[count] != '\r' && str[count] != '\0' && str[count] != '#') {
                    property[propertyCount++] = str[count];
                } else {
                    if (str[count] == '\0') {
                        isCompleted = true;
                    } else if (str[count] == '#') {
                        state = annotation;
                    } else {
                        // CRLF
                        if (str[count] == '\r' && str[count + 1] == '\n') {
                            count++;
                        }

                        state = ready;
                    }

                    // end of value
                    property[propertyCount] = '\0';

                    // remove blank
                    while (isBlank(property[--propertyCount])) {
                        property[propertyCount] = '\0';
                    }
                    node.setValue(property);
                    set(node.getKey(), node.getValue());
                }
                break;

            case annotation:
                if (str[count] == '\n' || str[count] == '\r') {
                    // CRLF
                    if (str[count] == '\r' && str[count + 1] == '\n') {
                        count++;
                    }
                    state = ready;
                }
                break;

            case error:
                property[propertyCount] = '\0';
                printf("Properties::analyze: syntax error -> [%s]\n", property);
                state = ready;
                break;
        }
        count++;
    }

    delete[] property;
}

void Properties::append(Node *node) {
    int index = node->hashCode() % ARRAY_SIZE;
    Node *linkRoot = properties[index];

    // first node is nullptr
    if (linkRoot == nullptr) {
        properties[index] = node;
    }
        // add node to the tail
    else {
        Node *inode = linkRoot;
        while (inode->getNext() != nullptr) {
            inode = inode->getNext();
        }
        inode->setNext(node);
        node->setPrevious(inode);
    }

    propertySize++;
}

Properties::Node *Properties::find(const char *key) {
    Node node(key, nullptr);
    int index = node.hashCode() % ARRAY_SIZE;

    Node *linkNode = properties[index];
    while (linkNode != nullptr) {
        // find the node
        if (linkNode->hashCode() == node.hashCode() &&
            linkNode->keyEquals(node.getKey())) {
            return linkNode;
        } else {
            linkNode = linkNode->getNext();
        }
    }

    // not found
    return nullptr;
}

/* Public */
bool Properties::load(const char *path) {
    if (path == nullptr) {
        puts("Properties::load: file path is null.");
        return false;
    }

    FILE *file = fopen(path, "rb");
    if (file == nullptr) {
        printf("Properties::load: open file failed.[%s]\n", path);
        return false;
    }

    char *buffer = readFile(file);
    fclose(file);
    if (buffer == nullptr) {
        // printf("Properties::load: read file failed.[%s]\n", path);
        return true;
    }

    analyze(buffer);
    return true;
}

bool Properties::save(const char *path) {
    if (path == nullptr) {
        puts("Properties::save: file path is null pointer.");
        return false;
    }

    FILE *file = fopen(path, "w+");
    if (file == nullptr) {
        printf("Properties::save: open/create file failed.[%s]\n", path);
        return false;
    }

    Node *node;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        node = properties[i];
        while (node != nullptr) {
            fprintf(file, "%s=%s\n", node->getKey(), node->getValue());
            node = node->getNext();
        }
    }
    fclose(file);
    return true;
}

int Properties::size() const {
    return propertySize;
}

void Properties::clear() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        Node *current = properties[i],
                *next;

        while (current) {
            next = current->getNext();
            delete current;
            current = next;
        }
    }
}

void Properties::remove(const char *key) {
    Node *target = find(key);

    // not found
    if (target == nullptr) {
        return;
    }

    // first node
    if (target->getPrevious() == nullptr) {
        properties[target->hashCode() % ARRAY_SIZE] = target->getNext();
    } else {
        target->getPrevious()->setNext(target->getNext());
    }
    delete target;
}

void Properties::set(const char *key, const char *value) {
    Node *target = find(key);

    // insert
    if (target == nullptr) {
        append(new Node(key, value));
    }
        // alter
    else {
        target->setValue(value);
    }
}

char *Properties::get(const char *key) {
    Node *target = find(key);
    return target == nullptr ? nullptr : target->getValue();
}

bool Properties::isInitSuccess() const {
    return initSuccess;
}

/***************************
 *    Properties::Node    *
 ***************************/

/* Construct */
Properties::Node::Node() {
    this->keyHashCode = 0;
    this->key = nullptr;
    this->value = nullptr;
    this->next = nullptr;
    this->previous = nullptr;
}

Properties::Node::Node(const char *key, const char *value) {
    this->keyHashCode = hash(key);
    this->key = stringCopy(key);
    this->value = stringCopy(value);
    this->next = nullptr;
    this->previous = nullptr;
}

Properties::Node::~Node() {
    delete[] key;
    delete[] value;
}

/* Private */
int Properties::Node::hash(const char *value) {
    int result = 0;

    if (value != nullptr) {
        int count = 0;
        char character;
        // '\0' -> 0
        while (0 != (character = value[count++])) {
            result = result * 31 + character;
        }
    }

    return result < 0 ? -result : result;
}

bool Properties::Node::stringCompare(const char *src, const char *dst) {
    int ret = 0;
    while (!(ret = *(unsigned char *) src - *(unsigned char *) dst) && *dst) {
        ++src, ++dst;
    }

    if (ret < 0) {
        ret = -1;
    } else if (ret > 0) {
        ret = 1;
    }
    return ret;
}

char *Properties::Node::stringCopy(const char *src) {
    if (src == nullptr) {
        return nullptr;
    }

    int length = 0;
    const char *str = src;
    while (*str != '\0') {
        length++, str++;
    }

    char *dest = new char[length + 1];

    dest[length] = '\0';
    for (int i = 0; i < length; i++) {
        dest[i] = src[i];
    }

    return dest;
}

/* Public */
void Properties::Node::setKey(const char *k) {
    delete[] this->key;
    this->key = stringCopy(k);
}

void Properties::Node::setValue(const char *v) {
    delete[] this->value;
    this->value = stringCopy(v);
}

void Properties::Node::setNext(Properties::Node *node) {
    delete this->next;
    this->next = node;
}

void Properties::Node::setPrevious(Properties::Node *node) {
    delete this->previous;
    this->previous = node;
}

char *Properties::Node::getKey() {
    return key;
}

char *Properties::Node::getValue() {
    return value;
}

Properties::Node *Properties::Node::getNext() {
    return next;
}

Properties::Node *Properties::Node::getPrevious() {
    return previous;
}

int Properties::Node::hashCode() const {
    return keyHashCode;
}

bool Properties::Node::equals(Node *other) {
    return this->hashCode() == other->hashCode() &&
           0 == stringCompare(this->getKey(), other->getKey()) &&
           0 == stringCompare(this->getValue(), other->getValue());
}

bool Properties::Node::keyEquals(const char *k) {
    return 0 == stringCompare(this->getKey(), k);
}